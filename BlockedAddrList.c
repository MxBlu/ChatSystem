//
// Created by mxblue on 17/04/17.
//

#include <stdlib.h>
#include <time.h>
#include "BlockedAddrList.h"
#include "VoidList.h"
#include "UserList.h"

struct BlockedAddrRep {
    VoidList list;
    int block_duration;
};

struct BlockedAddrNode {
    in_addr_t address;
    struct timespec timeoutStart;
};


void CheckBlockTimeouts(BlockedAddrList L);

BlockedAddrList newBlockedAddrList(int block_duration) {
    BlockedAddrList L = malloc(sizeof(struct BlockedAddrRep));
    L->list = newVoidList();
    L->block_duration = block_duration;

    return L;
}

void freeBlockedAddrList(BlockedAddrList L) {
    freeVoidList(L->list);
    free(L);
}

void BlockAddr(BlockedAddrList L, in_addr_t clientAddr) {
    struct BlockedAddrNode n;
    clock_gettime(CLOCK_MONOTONIC, &n.timeoutStart);
    n.address = clientAddr;

    VoidListInsert(L->list, &n, sizeof(struct BlockedAddrNode));
}
int AddrIsBlocked(BlockedAddrList L, in_addr_t clientAddr) {
    CheckBlockTimeouts(L);

    int it = VOIDLIST_ITERTORINIT;
    struct VoidNode *n = VoidListIterate(L->list, &it);
    while (it != VOIDLIST_ITERTOREND) {
        struct BlockedAddrNode *b = (struct BlockedAddrNode *)n->data;
        if (b->address == clientAddr)
            return 1;

        n = VoidListIterate(L->list, &it);
    }

    return 0;
}

void CheckBlockTimeouts(BlockedAddrList L) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    int it = VOIDLIST_ITERTORINIT;
    struct VoidNode *n = VoidListIterate(L->list, &it);
    while (it != VOIDLIST_ITERTOREND) {
        struct BlockedAddrNode *b = (struct BlockedAddrNode *)n->data;
        if (timeSubtract(now, b->timeoutStart) >= L->block_duration) {
            VoidListDelete(L->list, b);
            it--;
        }

        n = VoidListIterate(L->list, &it);
    }
}