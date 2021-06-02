//
// Created by Jumail on 23/03/2017.
//

#ifndef ASS1_CLIENTLIST_H
#define ASS1_CLIENTLIST_H

#include "Socket.h"
#include "UserList.h"

typedef struct ClientRep *ClientList;

typedef struct ClientNode {
    Socket clientfd;
    in_addr_t addr;
    User *user;
    int state;

    struct timespec lastMsgRecv;
    struct timespec lastHeartBeatRecv;
    int heartBeatSent;
} Client;

ClientList newClientList();
void freeClientList(ClientList L);

void insertClient(ClientList L, Socket clientfd, in_addr_t clientAddr);
void deleteClient(ClientList L, Client *c);

Client *getClient(ClientList L, User *user);
Client *getNextClient(ClientList L);

#endif //ASS1_CLIENTLIST_H
