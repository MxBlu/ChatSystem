//
// Created by mxblue on 14/04/17.
//

#ifndef ASS1_MESSAGELIST_H
#define ASS1_MESSAGELIST_H

#include "VoidList.h"

#define MESSAGELIST_ITERATORINIT VOIDLIST_ITERTORINIT
#define MESSAGELIST_ITERATOREND VOIDLIST_ITERTOREND

typedef struct MessageRep *MessageList;

typedef struct MessageNode {
    char *msg;
} Message;

MessageList newMessageList();
void freeMessageList(MessageList L);

void addMessage(MessageList L, char *msg);
void deleteMessage(MessageList L, Message *msg);

Message *MessageListTop(MessageList L);
Message *MessageListIterator(MessageList L, int *it);

int MessageListSize(MessageList L);

#endif //ASS1_MESSAGELIST_H
