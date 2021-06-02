//
// Created by mxblue on 14/04/17.
//

#include <stdlib.h>
#include <string.h>

#include "MessageList.h"
#include "VoidList.h"

struct MessageRep {
    VoidList list;
};

MessageList newMessageList() {
    MessageList L = malloc(sizeof(struct MessageRep));
    L->list = newVoidList();

    return L;
}

void freeMessageList(MessageList L) {
    int it = MESSAGELIST_ITERATORINIT;
    Message *m = MessageListIterator(L, &it);
    while (it != MESSAGELIST_ITERATOREND) {
        free(m->msg);

        m = MessageListIterator(L, &it);
    }


    freeVoidList(L->list);
    free(L);
}

void addMessage(MessageList L, char *msg) {
    Message m;
    m.msg = malloc(strlen(msg) + 1);
    strcpy(m.msg, msg);

    VoidListInsert(L->list, &m, sizeof(Message));
}

void deleteMessage(MessageList L, Message *msg) {
    free(msg->msg);
    VoidListDelete(L->list, msg);
}

Message *MessageListTop(MessageList L) {
    struct VoidNode *n = VoidListGet(L->list, 0);
    if (n == NULL) return NULL;

    return (Message *)n->data;
}

Message *MessageListIterator(MessageList L, int *it) {
    struct VoidNode *n = VoidListIterate(L->list, it);
    Message *m = (n != NULL) ? (Message *)n->data : NULL;

    return m;
}

int MessageListSize(MessageList L) {
    return VoidListLength(L->list);
}