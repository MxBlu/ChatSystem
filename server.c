//
// Created by Jumail on 20/03/2017.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <assert.h>

#include "Socket.h"
#include "ChatProtocol.h"
#include "UserList.h"
#include "ClientList.h"
#include "clientStates.h"
#include "BlockedAddrList.h"

#define NONAUTH_TIMEOUT 45 * 1000 // Timeout for non-authenticated users in milliseconds
#define HEARTBEAT_TIMEOUT 45 * 1000 // Timeout for authenticated users in milliseconds
#define HEARTBEAT_SEPARATION 25 * 1000 // Time without hearing from client to send a heartbeat in milliseconds

int main(int argc, char *argv[]) {
    // Check argument count
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [port] [block_duration] [timeout]\n", argv[0]);
        return -1;
    }

    // Ensure SIGPIPE doesn't crash the program
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, 0);

    // Convert and check for valid parameters
    int inactivity_timeout = atoi(argv[3]) * 1000;
    assert(inactivity_timeout >= 0);
    int block_duration = atoi(argv[2]) * 1000;
    assert(block_duration >= 0);
    int port = atoi(argv[1]);
    assert(port >= 0);

    // Create and set up server socket
    Socket serverfd = createSocket((unsigned short) port);
    if (serverfd <= 0) {
        printf("Error creating listening socket.\n");
        return -1;
    }
    setNonblocking(serverfd);

    // Create helper lists
    UserList users = newUserList("credentials.txt", block_duration);
    BlockedAddrList blockedAddrs = newBlockedAddrList(block_duration);
    ClientList clients = newClientList();

    // Set up time struct
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    // Run until program termination
    while (1) {
        int errState = 0;
        clock_gettime(CLOCK_MONOTONIC, &now);

        // Check for a new client
        in_addr_t clientAddr;
        Socket clientfd = getClientSocket(serverfd, &clientAddr);
        if (clientfd > 0) {
            setNonblocking(clientfd);
            insertClient(clients, clientfd, clientAddr);
        }

        // Get the next client to service
        Client *client = getNextClient(clients);
        if (client == NULL) continue;

        // Handle client inital state
        if (client->state == CLIENTSTATE_INIT) {
            // Check is address is blocked otherwise, ask for authentication
            if (AddrIsBlocked(blockedAddrs, client->addr)) {
                sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0);
                deleteClient(clients, client);
            } else {
                // Send authentication request and delete client if failed
                if (sendChatHeader(client->clientfd, CHAT_INTENT_AUTHREQUIRED, 0))
                    client->state = CLIENTSTATE_AUTHWAIT;
                else
                    deleteClient(clients, client);
            }
            continue;
        }

        // Get a packet from the client
        ChatPacket request = recvChatPacket(client->clientfd);

        // If the packet contains data, update times and heartbeat status
        if (request.intent != CHAT_INTENT_NULL) {
            if (client->user != NULL)
                client->user->lastOnline = now;

            client->lastHeartBeatRecv = now;
            client->heartBeatSent = 0;
            if (client->state == CLIENTSTATE_CMDWAIT && request.intent != CHAT_INTENT_HEARTBEAT)
                client->lastMsgRecv = now;
        }

        // Handle authentication
        if (request.intent == CHAT_INTENT_AUTHENTICATE) {
            // Extract username and password from packet and attempt authentication
            char *username = request.data;
            char *pass = &request.data[strlen(request.data) + 1];
            User *user = getUser(users, username);
            int auth = (user != NULL) ? UserAuthenticate(users, user, pass) : 0;

            // On success, handle user status and inform client
            if (auth == USERLIST_AUTHSUCCESS) {
                // Check if given user is already logged in
                if (!user->isOnline) {
                    // Inform client they are authenticated
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_SUCCESS, 0)) errState = ERRSTATE_CONNECTIONCLOSED;

                    // Update client and user states
                    client->state = CLIENTSTATE_CMDWAIT;
                    client->user = user;
                    user->isOnline = USERLIST_ONLINE;
                    user->lastOnline = now;

                    // Inform all online (and not blocked) users that the user has logged in
                    int msgLen = strlen(user->username) + 26;
                    char *msg = malloc(msgLen + 1);
                    snprintf(msg, msgLen, "<server>: %s has logged in.", user->username);
                    int it = USERLIST_ITERATORINIT;
                    User *currUser = UserListIterator(users, &it);
                    while (it != USERLIST_ITERATOREND) {
                        if (currUser != user && currUser->isOnline && !UserIsBlocked(client->user, currUser))
                            addMessage(currUser->messages, msg);

                        currUser = UserListIterator(users, &it);
                    }
                    free(msg);
                } else {
                    sendChatHeader(client->clientfd, CHAT_INTENT_ALREADY, 0);
                }
            // If user is login blocked, inform client
            } else if (auth == USERLIST_AUTHBLOCKED) {
                sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0);
                errState = ERRSTATE_USERBLOCKED;
            // If given an invalid username, IP-block the client
            } else if (user == NULL) {
                BlockAddr(blockedAddrs, client->addr);
                sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0);
                errState = ERRSTATE_USERBLOCKED;
            // If password is incorrect, inform the client
            } else {
                if (!sendChatHeader(client->clientfd, CHAT_INTENT_AUTHREQUIRED, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
            }
        // Handle client-requested disconnect
        } else if (request.intent == CHAT_INTENT_DISCONNECT) {
            // Flush the socket
            shutdown(client->clientfd, SHUT_WR);
            errState = ERRSTATE_QUITNOW;
        // Handle messaging
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_PM) {
            // Extract message and target user
            char *user = request.data;
            char *msg = &request.data[strlen(user) + 1];
            User *target = getUser(users, user);

            // If target is valid (i.e. exists and is not self), attempt to queue the message to the user
            if (target != NULL && target != client->user) {
                if (!UserIsBlocked(target, client->user)) {
                    // Reformat the message into "<[user]>: [message]" format and add to user message queue
                    int formattenLen = strlen(msg) + strlen(client->user->username) + 4;
                    char *formattedMsg = malloc(formattenLen + 1);
                    snprintf(formattedMsg, formattenLen + 1, "<%s>: %s", client->user->username, msg);
                    addMessage(target->messages, formattedMsg);
                    free(formattedMsg);
                // If target user has blocked source user, inform client
                } else {
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_BLOCKUSR, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
                }
            // If given an invalid user, inform client
            } else {
                if (!sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
            }
        // Handle broadcasts
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_BROADCAST) {
            // Extract and reformat broadcast message into "<[user]>: [message]" format
            int formattedLen = strlen(request.data) + strlen(client->user->username) + 4;
            char *formattedMsg = malloc(formattedLen + 1);
            snprintf(formattedMsg, formattedLen + 1, "<%s>: %s", client->user->username, request.data);

            // Iterate over all users and queue message to online users that are not blocked and not self
            int it = USERLIST_ITERATORINIT;
            int numBlocked = 0;
            User *user = UserListIterator(users, &it);
            while (it != USERLIST_ITERATOREND) {
                if (user->isOnline && user != client->user) {
                    if (!UserIsBlocked(user, client->user))
                        addMessage(user->messages, formattedMsg);
                    else
                        numBlocked++;
                }

                user = UserListIterator(users, &it);
            }
            free(formattedMsg);

            // If any user blocked the broadcast, inform the user
            if (numBlocked > 0) {
                if (!sendChatHeader(client->clientfd, CHAT_INTENT_BLOCKUSR, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
            }
        // Handle user blocking
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_BLOCKUSR) {
            // Get target user from packet
            User *target = getUser(users, request.data);

            // Check user exists and is not already blocked, informing client of command status
            if (target != NULL && target != client->user) {
                if (UserBlock(client->user, target)) {
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_SUCCESS, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
                } else {
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_ALREADY, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
                }
            } else {
                if (!sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
            }
        // Handle user unblocking
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_UNBLOCKUSR) {
            // Get target user from packet
            User *target = getUser(users, request.data);

            // Check if user exists and is not already unblocked, informing client of command status
            if (target != NULL && target != client->user) {
                if (UserUnblock(client->user, target)) {
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_SUCCESS, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
                } else {
                    if (!sendChatHeader(client->clientfd, CHAT_INTENT_ALREADY, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
                }
            } else {
                if (!sendChatHeader(client->clientfd, CHAT_INTENT_FAILED, 0)) errState = ERRSTATE_CONNECTIONCLOSED;
            }
        // Handle user presence request
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_WHOELSE) {
            // Create array to store message string
            char whoelse[10000];
            whoelse[0] = '\0';

            // Iterate over all users and add online users to the message string
            int it = USERLIST_ITERATORINIT;
            User *user = UserListIterator(users, &it);
            while (it != USERLIST_ITERATOREND) {
                if (user->isOnline && user != client->user) {
                    strcat(whoelse, user->username);
                    strcat(whoelse, "\n");
                }

                user = UserListIterator(users, &it);
            }

            // Send message string to client
            if (!sendChatPacket(client->clientfd, CHAT_INTENT_WHOELSE, whoelse, strlen(whoelse) + 1)) errState = ERRSTATE_CONNECTIONCLOSED;
        // Handle checking users online in a given time period
        } else if (client->state == CLIENTSTATE_CMDWAIT && request.intent == CHAT_INTENT_WHOELSESINCE) {
            // Extract time from packet
            int time = *(int *)request.data;

            // Create array to store message string
            char whoelse[10000];
            whoelse[0] = '\0';

            // Iterate over users and add users that were online in the given period to the message string
            int it = USERLIST_ITERATORINIT;
            User *user = UserListIterator(users, &it);
            while (it != USERLIST_ITERATOREND) {
                if ((user->isOnline || (timeSubtract(now, user->lastOnline)/1000) <= time) && user != client->user) {
                    strcat(whoelse, user->username);
                    strcat(whoelse, "\n");
                }

                user = UserListIterator(users, &it);
            }

            // Send message string to client
            if (!sendChatPacket(client->clientfd, CHAT_INTENT_WHOELSE, whoelse, strlen(whoelse) + 1)) errState = ERRSTATE_CONNECTIONCLOSED;
        // Flush messages for the client user to the client (guaranteed before disconnect)
        } else if (client->state == CLIENTSTATE_CMDWAIT && MessageListSize(client->user->messages) > 0) {
            // Send and delete every message on the client user's queue
            MessageList messages = client->user->messages;
            while (MessageListSize(messages) > 0) {
                Message *m = MessageListTop(messages);
                if (!sendChatPacket(client->clientfd, CHAT_INTENT_PM, m->msg, strlen(m->msg) + 1)) errState = ERRSTATE_CONNECTIONCLOSED;

                deleteMessage(messages, m);
            }
        }

        // Set the amount of time passed since last message from the client
        int deltaTime = timeSubtract(now, client->lastHeartBeatRecv);

        // Handle timeouts
        // If client is logged in and no packet has been received in HEARTBEAT_TIMEOUT seconds
        if (deltaTime > HEARTBEAT_TIMEOUT && client->state != CLIENTSTATE_AUTHWAIT) {
            sendChatHeader(client->clientfd, CHAT_INTENT_CLIENTTIMEDOUT, 0);
            errState = ERRSTATE_USERTIMEOUT;
        //If client has not authenticated in NONAUTH_TIMEOUT seconds
        } else if (deltaTime > NONAUTH_TIMEOUT && client ->state == CLIENTSTATE_AUTHWAIT) {
            sendChatHeader(client->clientfd, CHAT_INTENT_CLIENTTIMEDOUT, 0);
            errState = ERRSTATE_AUTHTIMEOUT;
        // If user has not sent a valid command in inactivity_timeout seconds
        } else if (timeSubtract(now, client->lastMsgRecv) > inactivity_timeout && client->state != CLIENTSTATE_AUTHWAIT) {
            sendChatHeader(client->clientfd, CHAT_INTENT_DISCONNECT, 0);
            errState = ERRSTATE_USERTIMEOUT;
        }

        // If HEARTBEAT_SEPEARTION seoconds have passed without a message from the client,
        // send a heartbeat packet
        if (deltaTime > HEARTBEAT_SEPARATION && client->state != CLIENTSTATE_AUTHWAIT && !client->heartBeatSent) {
            sendChatHeader(client->clientfd, CHAT_INTENT_HEARTBEAT, 0);
            client->heartBeatSent = 1;
        }

        // Handle any reason to disconnect and cleanup the client
        if (errState > 0) {
            // If the user was logged in, inform all online (and not blocked) users that the user has logged off
            if (client->user != NULL) {
                int msgLen = strlen(client->user->username) + 27;
                char *msg = malloc(msgLen + 1);
                snprintf(msg, msgLen, "<server>: %s has logged out.", client->user->username);

                int it = USERLIST_ITERATORINIT;
                User *currUser = UserListIterator(users, &it);
                while (it != USERLIST_ITERATOREND) {
                        if (currUser != client->user && currUser->isOnline && !UserIsBlocked(client->user, currUser))
                        addMessage(currUser->messages, msg);

                    currUser = UserListIterator(users, &it);
                }
                free(msg);

                // Set user to offline
                client->user->isOnline = USERLIST_OFFLINE;
            }
            // Remove client from client list
            deleteClient(clients, client);
        }
    }

    // Clean up helper lists
    freeClientList(clients);
    freeUserList(users);
    freeBlockedAddrList(blockedAddrs);

    // Clean up server listening socket
    closeSocket(serverfd);
    return 0;
}