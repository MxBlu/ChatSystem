// LinkedList.c, list of username-passwords
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#ifndef CREDLIST_H
#define CREDLIST_H

#include <stdio.h>

#define CREDLIST_AUTHSUCCESS 1
#define CREDLIST_AUTHFAILED 0

typedef struct CredentialRep *CredentialList;

CredentialList newCredentialList(char *);
int CredentialCheck(CredentialList L, char *user, char *pass);
void freeCredentialList(CredentialList L);

#endif
