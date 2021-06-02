#include <stdio.h>
#include <stdlib.h>

#include "Socket.h"
#include "ChatProtocol.h"
#include "clientStates.h"

#define CHAT_MAXCMDSIZE 10000

void sendAuth(Socket serverfd);
void sendMsg(Socket serverfd, char *msg);
void sendBroadcast(Socket serverfd, char *msg);
void sendBlock(Socket serverfd, char *user);
void sendUnblock(Socket serverfd, char *user);
void sendWhoelsesince(Socket serverfd, char *time);

void printHelp();

//stdin clearing code from: http://c-faq.com/stdio/stdinflush2.html
void clearInput();
// kbhit() code from: http://www.ozzu.com/unix-linux-forum/kbhit-t34401.html
int kbhit(void);

// Global error state
int errState = 0;

int main(int argc, char *argv[]) {
    // Check argument count
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [hostname] [port]\n", argv[0]);
        return -1;
    }

    int state = CLIENTSTATE_INIT;
    Socket serverfd = 0;

    // Attempt to connect to the given server
    int i = 0;
    for (; i < 3; i++) {
        if ((serverfd = connectTo(argv[1], argv[2])) > 0) break;
        printf("Retrying connection %d/3 times...\n", i + 1);
        Sleep(3000);
    } if (i == 3) return -1;

    // Set server socket to non-blocking
    setNonblocking(serverfd);
    printf("Connected to server.\n");

    char cmdBuff[CHAT_MAXCMDSIZE];
    int lastCmd = CHAT_INTENT_NULL;

    // Loop until break
    while (1) {
        // Wait for a packet from the server or keyboard interrupt (Enter press) from user
        ChatPacket data;
        data.intent = CHAT_INTENT_NULL;
        while (data.intent == CHAT_INTENT_NULL && state != CLIENTSTATE_CMDPROCESS) {
            data = recvChatPacket(serverfd);

            // If a key is pressed, stop waiting for packet and get user command
            if (kbhit() && state == CLIENTSTATE_CMDWAIT) {
                printf("> ");
                clearInput();
                fgets(cmdBuff, CHAT_MAXCMDSIZE, stdin);
                cmdBuff[strlen(cmdBuff) - 1] = '\0';
                state = CLIENTSTATE_CMDPROCESS;
            }

            // Attempt to not busy wait
            Sleep(50);
        }

        // Handle server timing the client out (no message)
        if (data.intent == CHAT_INTENT_CLIENTTIMEDOUT) {
            printf("Connection timed out.\n");
            errState = ERRSTATE_CONNECTIONCLOSED;

        // Handle client initial state
        } else if (state == CLIENTSTATE_INIT) {
            // Attempt to authenticate the client
            if (data.intent == CHAT_INTENT_AUTHREQUIRED) {
                state = CLIENTSTATE_AUTHWAIT;
                sendAuth(serverfd);
            // Inform user that their IP is currently blocked
            } else if (data.intent == CHAT_INTENT_FAILED) {
                printf("Your IP is currently blocked, try again later.\n");
                errState = ERRSTATE_USERBLOCKED;
            }

        // Handle authentication
        } else if (state == CLIENTSTATE_AUTHWAIT) {
            // Incorrect password was specified
            if (data.intent == CHAT_INTENT_AUTHREQUIRED) {
                printf("Login failed, try again.\n");
                sendAuth(serverfd);
            // Specified user is currently blocked
            } else if (data.intent == CHAT_INTENT_FAILED) {
                printf("User is invalid or blocked, try again later.\n");
                errState = ERRSTATE_USERBLOCKED;
            // User was successfully authenticated
            } else if (data.intent == CHAT_INTENT_SUCCESS) {
                printf("\nLogin successful.\n"
                       "Press enter to input a command.\n"
                       "\"help\" lists all available commands.\n\n");
                state = CLIENTSTATE_CMDWAIT;
            // Specified user is already logged in
            } else if (data.intent == CHAT_INTENT_ALREADY) {
                printf("Username is currently logged in, try again later.\n");
                errState = ERRSTATE_USERBLOCKED;
            }

        // Handle user commands
        } else if (state == CLIENTSTATE_CMDPROCESS) {
            // Disconnect from the server
            if (strcmp(cmdBuff, "logout") == 0) {
                sendChatHeader(serverfd, CHAT_INTENT_DISCONNECT, 0);
                errState = ERRSTATE_QUITNOW;
            // Send a message to a user
            } else if (strncmp(cmdBuff, "message", 7) == 0) {
                sendMsg(serverfd, &cmdBuff[7]);
                lastCmd = CHAT_INTENT_PM;
            // Send a message to all online users
            } else if (strncmp(cmdBuff, "broadcast", 9) == 0) {
                sendBroadcast(serverfd, &cmdBuff[9]);
                lastCmd = CHAT_INTENT_BROADCAST;
            // Block a user
            } else if (strncmp(cmdBuff, "block", 5) == 0) {
                sendBlock(serverfd, &cmdBuff[5]);
                lastCmd = CHAT_INTENT_BLOCKUSR;
            // Unblock a user
            } else if (strncmp(cmdBuff, "unblock", 7) == 0) {
                sendUnblock(serverfd, &cmdBuff[7]);
                lastCmd = CHAT_INTENT_UNBLOCKUSR;
            // Get who else is online
            } else if (strcmp(cmdBuff, "whoelse") == 0) {
                sendChatHeader(serverfd, CHAT_INTENT_WHOELSE, 0);
                lastCmd = CHAT_INTENT_WHOELSE;
            // Get who else has been online since a given time
            } else if (strncmp(cmdBuff, "whoelsesince", 12) == 0) {
                sendWhoelsesince(serverfd, &cmdBuff[12]);
                lastCmd = CHAT_INTENT_WHOELSESINCE;
            // Print available commands
            } else if (strcmp(cmdBuff, "help") == 0) {
                printHelp();
            // Handle any other user input
            } else {
                printf(">> Invalid command.\n");
            }

            state = CLIENTSTATE_CMDWAIT;
        // Handle server messages
        } else if (state == CLIENTSTATE_CMDWAIT) {
            // Print a endline terminated message
            if (data.intent == CHAT_INTENT_PM) {
                printf("%s\n", data.data);
            // Print a list of users
            } else if (data.intent == CHAT_INTENT_WHOELSE) {
                printf("%s", data.data);
            // Handle a command-specific failure message from the server
            } else if (data.intent == CHAT_INTENT_FAILED) {
                if (lastCmd == CHAT_INTENT_PM)
                    printf(">> Invalid user.\n");
                else if (lastCmd == CHAT_INTENT_BLOCKUSR)
                    printf(">> Invalid User.\n");
                else if (lastCmd == CHAT_INTENT_UNBLOCKUSR)
                    printf(">> Invalid User.\n");
            // Handle semi-failure messages for block/unblock from the server
            } else if (data.intent == CHAT_INTENT_ALREADY) {
                if (lastCmd == CHAT_INTENT_BLOCKUSR)
                    printf(">> User already blocked.\n");
                else if (lastCmd == CHAT_INTENT_UNBLOCKUSR)
                    printf(">> User already unblocked.\n");
            // Handle success messages for block/unblock from the server
            } else if (data.intent == CHAT_INTENT_SUCCESS) {
                if (lastCmd == CHAT_INTENT_BLOCKUSR)
                    printf(">> User blocked.\n");
                else if (lastCmd == CHAT_INTENT_UNBLOCKUSR)
                    printf(">> User unblocked.\n");
            // Handle partially or completely blocked messages from the server
            } else if (data.intent == CHAT_INTENT_BLOCKUSR) {
                if (lastCmd == CHAT_INTENT_PM) {
                    printf(">> Message was blocked.\n");
                } else if (lastCmd == CHAT_INTENT_BROADCAST) {
                    printf(">> Message was blocked by some users.\n");
                }
            // Handle server heartbeat requests
            } else if (data.intent == CHAT_INTENT_HEARTBEAT) {
                sendChatHeader(serverfd, CHAT_INTENT_HEARTBEAT, 0);
            // Handle disconnects due to inactivity from the server
            } else if (data.intent == CHAT_INTENT_DISCONNECT) {
                printf("User disconnected due to inactivity.\n");
                errState = ERRSTATE_USERTIMEOUT;
            }
        }

        // Clean up received packet
        deleteChatPacket(data);

        // Check for any reason to close the connection
        if (errState > 0)
            break;
    }

    // Flush any packets in the socket and clean up
    shutdown(serverfd, SHUT_RDWR);
    closeSocket(serverfd);
    return 0;
}

void sendAuth(Socket serverfd) {
    char credentials[1000];

    printf("Please enter your login details.\n");
    printf("Username: ");
    scanf("%s", credentials);
    int userLen = strlen(credentials) + 1;
    printf("Password: ");
    scanf("%s", &credentials[userLen]);
    int authLen = userLen + strlen(&credentials[userLen]) + 1;

    if(!sendChatPacket(serverfd, CHAT_INTENT_AUTHENTICATE, credentials, authLen)) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }
    clearInput();
}

void sendMsg(Socket serverfd, char *msg) {
    if (strncmp(msg, " ", 1) != 0) {
        printf(">> Invalid user.\n");
        return;
    }
    msg++;

    char *message = strstr(msg, " ");
    if (message == NULL) {
        printf(">> Invalid message.\n");
        return;
    }

    int userLen = (int) (message - msg);
    char *user = malloc(userLen + 1);
    strncpy(user, msg, userLen);

    message++;
    if (*message == '\0') {
        printf(">> Invalid message.\n");
        free(user);
        return;
    }

    int msgLen = strlen(message);

    char *packetData = malloc(userLen + msgLen + 2);
    strcpy(packetData, user);
    packetData[userLen] = '\0';
    strcpy(&packetData[userLen + 1], message);

    if(!sendChatPacket(serverfd, CHAT_INTENT_PM, packetData, userLen + msgLen + 2)) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }

    free(user);
    free(packetData);
}

void sendBroadcast(Socket serverfd, char *msg) {
    if (strncmp(msg, " ", 1) != 0) {
        printf(">> Invalid message.\n");
        return;
    }
    msg++;

    if(!sendChatPacket(serverfd, CHAT_INTENT_BROADCAST, msg, strlen(msg) + 1)) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }
}

void sendBlock(Socket serverfd, char *user) {
    if (strncmp(user, " ", 1) != 0) {
        printf(">> Invalid user.\n");
        return;
    }
    user++;

    if(!sendChatPacket(serverfd, CHAT_INTENT_BLOCKUSR, user, strlen(user) + 1)) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }
}

void sendUnblock(Socket serverfd, char *user) {
    if (strncmp(user, " ", 1) != 0) {
        printf(">> Invalid user.\n");
        return;
    }
    user++;

    if(!sendChatPacket(serverfd, CHAT_INTENT_UNBLOCKUSR, user, strlen(user) + 1)) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }
}

void sendWhoelsesince(Socket serverfd, char *time) {
    if (strncmp(time, " ", 1) != 0) {
        printf(">> Invalid time.\n");
        return;
    }
    time++;

    int t = atoi(time);
    if (t <= 0) {
        printf(">> Invalid time.\n");
        return;
    }

    if(!sendChatPacket(serverfd, CHAT_INTENT_WHOELSESINCE, (char *)&t, sizeof(int))) {
        errState = ERRSTATE_CONNECTIONCLOSED;
    }
}

void printHelp() {
    printf(">> Available commands:\n"
           "\n"
           "logout                  Disconnects from the server.\n"
           "message <user> <msg>    Sends <msg> to <user>.\n"
           "broadcast <msg>         Sends <msg> to all online users.\n"
           "block <user>            Blocks <user>.\n"
           "unblock <user>          Unblocks <user>.\n"
           "whoelse                 Gets users that are online.\n"
           "whoelsesince <time>     Gets users that have been online since <time> seconds ago.\n"
           "help                    Displays this message.\n"
           "\n"
    );
}

int kbhit(void) {
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd, NULL, NULL, &tv) == -1)
        return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
}

void clearInput() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}