// LinkedList.c, list of username-passwords
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "assert.h"
#include "LinkedList.h"

struct ListNode {
	char *user;
    char *pass;
	struct ListNode *next;
};

struct ListRep {
	int  size;
	struct ListNode *first;
	struct ListNode *last;
};

List newList()
{
	struct ListRep *L;

	L = malloc(sizeof (struct ListRep));
	assert (L != NULL);
	L->size = 0;
	L->first = NULL;
	L->last = NULL;
	return L;
}

static void freeNode(struct ListNode *l) {
    free(l->user);
    free(l->pass);
    free(l);
}

static void freeNodeRecurse(struct ListNode *l) {
    if(l == NULL)
        return;

    freeNodeRecurse(l->next);
    freeNode(l);
}

void freeList(List L)
{
    freeNodeRecurse(L->first);
}

// create an IntList by reading values from a file
// assume that the file is open for reading
List getList(FILE *inf)
{
	List L;
	char user[100];
    char pass[100];

	L = newList();
	while (fscanf(inf,"%s %s",user, pass) != EOF)
        ListInsert(L, user, pass);
	return L;
}

static struct ListNode *newListNode(char *user, char *pass)
{
	struct ListNode *n;

	n = malloc(sizeof (struct ListNode));
	assert(n != NULL);
    n->user = malloc(strlen(user) + 1);
    n->pass = malloc(strlen(pass) + 1);
	n->next = NULL;

    strcpy(n->user, user);
    strcpy(n->pass, pass);

	return n;
}

void ListInsert(List L, char *user, char *pass)
{
	struct ListNode *n;

	assert(L != NULL);
	n = newListNode(user, pass);
	if (L->first == NULL)
		L->first = L->last = n;
	else {
		L->last->next = n;
		L->last = n;
	}
	L->size++;
}

void ListDelete(List L, char *user)
{
	struct ListNode *curr, *prev;

	assert(L != NULL);

	// find where v occurs in list
	prev = NULL; curr = L->first;
	while (curr != NULL && strcmp(curr->user, user) == 0) {
		prev = curr;
		curr = curr->next;
	}
	// not found; give up
	if (curr == NULL) return;
	// unlink curr
	if (prev == NULL)
		L->first = curr->next;
	else
		prev->next = curr->next;
	if (L->last == curr)
		L->last = prev;
	L->size--;
	// remove curr
	freeNode(curr);
}

int ListCheck(List L, char *user, char *pass) {
    assert(L != NULL);
    struct ListNode *curr = L->first;

    while(curr != NULL) {
        if (strcmp(curr->user, user) == 0) {
            if (strcmp(curr->pass, pass) == 0)
                return LIST_AUTHSUCCESS;
            else
                return LIST_AUTHFAILED;
        }

        curr = curr->next;
    }

    return LIST_AUTHFAILED;
}

int ListLength(List L)
{
	assert(L != NULL);
	return L->size;
}

List ListCopy(List L)
{
	struct ListRep *Lnew;
	struct ListNode *curr;

	Lnew = newList();
	for (curr = L->first; curr != NULL; curr = curr->next)
        ListInsert(Lnew, curr->user, curr->pass);
	return Lnew;
}