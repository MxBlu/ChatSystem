// LinkedList.c, list of username-passwords
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "VoidList.h"
#include "CredentialList.h"

struct CredentialRep {
	VoidList list;
};

CredentialList newCredentialList(char *file)
{
	struct CredentialRep *L;

	L = malloc(sizeof (struct CredentialRep));
    assert (L != NULL);

    L->list = newVoidList();
    FILE *credFile = fopen(file, "r");

    if (credFile == NULL) exit(-1);

    char user[100];
    char pass[100];

    while (fscanf(credFile,"%s %s\n", user, pass) != EOF)
        VoidListInsert(L->list, user, strlen(user) + 1, pass, strlen(pass) + 1);

	return L;
}

void freeCredentialList(CredentialList L) {
    freeVoidList(L->list);
    free(L);
}

int CredentialCheck(CredentialList L, char *user, char *pass) {
    assert(L != NULL);

    int passSize;
    char *storedPass = VoidListFind(L->list, user, strlen(user) + 1, &passSize, VOIDLIST_FIELD_ID);

    if (storedPass == NULL || strcmp(pass, storedPass))
        return CREDLIST_AUTHFAILED;
    else
        return CREDLIST_AUTHSUCCESS;
}