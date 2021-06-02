//
// Created by Jumail on 23/03/2017.
//

#include <stdlib.h>
#include <bits/time.h>
#include <time.h>
#include "clientStates.h"
#include "ClientList.h"
#include "VoidList.h"

struct ClientRep {
    VoidList list;
    int it;
};

ClientList newClientList() {
    ClientList L = malloc(sizeof(struct ClientRep));

    L->list = newVoidList();
    L->it = VOIDLIST_ITERTORINIT;
    return L;
}

void freeClientList(ClientList L) {
    int it = VOIDLIST_ITERTORINIT;

    struct VoidNode *n = VoidListIterate(L->list, &it);
    while (it != VOIDLIST_ITERTOREND) {
        Client *c = (Client *)n->data;
        closeSocket(c->clientfd);
        if (c->user != NULL)
            c->user->isOnline = USERLIST_OFFLINE;

        n = VoidListIterate(L->list, &it);
    }

    freeVoidList(L->list);
    free(L);
}

void insertClient(ClientList L, Socket clientfd, in_addr_t clientAddr) {
    Client c;
    c.user = NULL;
    c.clientfd = clientfd;
    c.addr = clientAddr;
    clock_gettime(CLOCK_MONOTONIC, &c.lastMsgRecv);
    c.lastHeartBeatRecv = c.lastMsgRecv;
    c.heartBeatSent = 0;
    c.state = CLIENTSTATE_INIT;

    VoidListInsert(L->list, &c, sizeof(Client));
}

void deleteClient(ClientList L, Client *c) {
    closeSocket(c->clientfd);
    if (c->user != NULL)
        c->user->isOnline = USERLIST_OFFLINE;

    VoidListDelete(L->list, c);
}

Client *getClient(ClientList L, User *user) {
    int it = VOIDLIST_ITERTORINIT;

    struct VoidNode *n = VoidListIterate(L->list, &it);

    while (it != VOIDLIST_ITERTOREND) {
        Client *c = (Client *)n->data;

        if (user == c->user)
            return c;

        n = VoidListIterate(L->list, &it);
    }

    return NULL;
}

Client *getNextClient(ClientList L) {
    struct VoidNode *n = VoidListIterate(L->list, &L->it);
    if (L->it == VOIDLIST_ITERTOREND) n = VoidListIterate(L->list, &L->it);
    if (n == NULL) return NULL;
    return (Client *)n->data;
}