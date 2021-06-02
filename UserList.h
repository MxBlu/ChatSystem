//
// Created by mxblue on 14/04/17.
//

#ifndef ASS1_USERLIST_H
#define ASS1_USERLIST_H

#include <time.h>

#include "VoidList.h"
#include "MessageList.h"

#define USERLIST_AUTHSUCCESS 1
#define USERLIST_AUTHFAILED 0
#define USERLIST_AUTHBLOCKED -1

#define USERLIST_OFFLINE 0
#define USERLIST_ONLINE 1

#define USERLIST_ITERATORINIT VOIDLIST_ITERTORINIT
#define USERLIST_ITERATOREND VOIDLIST_ITERTOREND

typedef struct BlockedRep *BlockedList;
typedef struct UserRep *UserList;

typedef struct UserNode {
    char *username;
    char *pass;

    int isOnline;
    MessageList messages;
    BlockedList blockedUsers;
    struct timespec lastOnline;

    int lastLoginAttmpts;
    struct timespec lastLoginAttmptTime;
} User;

UserList newUserList(char *userFile, int block_duration);
void freeUserList(UserList L);

User *getUser(UserList L, char *username);
User *UserListIterator(UserList L, int *it);

int UserAuthenticate(UserList L, User *user, char *pass);
int UserBlock(User *user, User *target);
int UserUnblock(User *user, User *target);
int UserIsBlocked(User *user, User *target);

int timeSubtract(struct timespec t1, struct timespec t2);
BlockedList newBlockedList();
void freeBlockedList(BlockedList L);

int addUser(BlockedList L, User *u);
int removeUser(BlockedList L, User *u);

int inList(BlockedList L, User *u);

int timeSubtract(struct timespec t1, struct timespec t2);

#endif //ASS1_USERLIST_H
