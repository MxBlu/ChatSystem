//
// Created by mxblue on 16/04/17.
//

#include <stdlib.h>
#include "UserList.h"

typedef struct BlockedNode {
    User *user;
} BlockedNode;

BlockedNode *BlockedListFind(BlockedList L, User *u);

struct BlockedRep {
    VoidList list;
};

BlockedList newBlockedList() {
    BlockedList L = malloc(sizeof(struct BlockedRep));
    L->list = newVoidList();

    return L;
}

void freeBlockedList(BlockedList L) {
    freeVoidList(L->list);
    free(L);
}

int addUser(BlockedList L, User *u) {
    if (inList(L, u))
        return 0;

    BlockedNode n;
    n.user = u;

    VoidListInsert(L->list, &u, sizeof(BlockedNode));

    return 1;
}

int removeUser(BlockedList L, User *u) {
    BlockedNode *b = BlockedListFind(L, u);
    if (b == NULL)
        return 0;

    VoidListDelete(L->list, b);
    return 1;
}

int inList(BlockedList L, User *u) {
    return (BlockedListFind(L, u) == NULL) ? 0 : 1;
}

BlockedNode *BlockedListFind(BlockedList L, User *u) {
    int it = VOIDLIST_ITERTORINIT;
    struct VoidNode *n = VoidListIterate(L->list, &it);
    while (it != VOIDLIST_ITERTOREND) {
        BlockedNode *b = (BlockedNode *)n->data;
        if (b->user == u)
            return b;

        n = VoidListIterate(L->list, &it);
    }

    return NULL;
}