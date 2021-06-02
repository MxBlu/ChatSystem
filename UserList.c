//
// Created by mxblue on 14/04/17.
//

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "UserList.h"
#include "VoidList.h"

struct UserRep {
    VoidList list;
    int block_duration;
};

UserList newUserList(char *userFile, int block_duration) {
    UserList L = malloc(sizeof (struct UserRep));
    assert (L != NULL);

    L->list = newVoidList();
    FILE *f = fopen(userFile, "r");

    assert(f != NULL);

    char user[500];
    char pass[500];
    struct timespec ts;
    ts.tv_nsec = 0;
    ts.tv_sec = 0;

    while (fscanf(f,"%s %s\n", user, pass) != EOF) {
        User n;
        n.username = malloc(strlen(user) + 1);
        n.pass = malloc(strlen(pass) + 1);

        n.isOnline = USERLIST_OFFLINE;
        n.lastOnline.tv_sec = 0;
        n.lastOnline.tv_nsec = 0;
        n.messages = newMessageList();
        n.blockedUsers = newBlockedList();

        n.lastLoginAttmpts = 0;
        n.lastLoginAttmptTime = ts;

        strcpy(n.username, user);
        strcpy(n.pass, pass);
        VoidListInsert(L->list, &n, sizeof(n));
    }

    L->block_duration = block_duration;
    return L;
}

void freeUserList(UserList L) {
    int it = USERLIST_ITERATORINIT;
    User *u = UserListIterator(L, &it);
    while (it != USERLIST_ITERATOREND) {
        free(u->username);
        free(u->pass);

        freeMessageList(u->messages);
        freeBlockedList(u->blockedUsers);

        UserListIterator(L, &it);
    }

    freeVoidList(L->list);
    free(L);
}

User *getUser(UserList L, char *username) {
    int it = USERLIST_ITERATORINIT;

    User *u = UserListIterator(L, &it);
    while (it != USERLIST_ITERATOREND) {
        if (!strcmp(username, u->username))
            return u;

        u = UserListIterator(L, &it);
    }

    return NULL;
}

User *UserListIterator(UserList L, int *it) {
    struct VoidNode *n = VoidListIterate(L->list, it);
    User *u = (n != NULL) ? (User *)n->data : NULL;

    return u;
}

int UserAuthenticate(UserList L, User *user, char *pass) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (timeSubtract(now, user->lastLoginAttmptTime) >= L->block_duration)
        user->lastLoginAttmpts = 0;

    if (user->lastLoginAttmpts == 3)
        return USERLIST_AUTHBLOCKED;
    if (strcmp(pass, user->pass)) {
        if (user->lastLoginAttmpts == 0)
            user->lastLoginAttmptTime = now;

        user->lastLoginAttmpts++;
        return USERLIST_AUTHFAILED;
    }

    user->lastLoginAttmpts = 0;
    return USERLIST_AUTHSUCCESS;
}

int UserBlock(User *user, User *target) {
    return addUser(user->blockedUsers, target);
}

int UserUnblock(User *user, User *target) {
    return removeUser(user->blockedUsers, target);
}

int UserIsBlocked(User *user, User *target) {
    return inList(user->blockedUsers, target);
}

int timeSubtract(struct timespec t1, struct timespec t2) {
    return (int) ((t1.tv_sec - t2.tv_sec) * 1000 + (t1.tv_nsec - t2.tv_nsec) / 1000000);
}