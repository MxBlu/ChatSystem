// Voidlist.c, void-pointer allocated list
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "VoidList.h"

struct VoidRep {
    int  size;
    struct VoidNode *first;
    struct VoidNode *last;
};

VoidList newVoidList() {
	struct VoidRep *L;

	L = malloc(sizeof (struct VoidRep));
	assert (L != NULL);
	L->size = 0;
	L->first = NULL;
	L->last = NULL;
	return L;
}

static void freeNode(struct VoidNode *n) {
    free(n->data);
    free(n);
}

static void freeNodeRecurse(struct VoidNode *n) {
    if(n == NULL)
        return;

    freeNodeRecurse(n->next);
    freeNode(n);
}

void freeVoidList(VoidList L) {
    freeNodeRecurse(L->first);
    free(L);
}

static struct VoidNode *newListNode(void *data, int size) {
	struct VoidNode *n;

	n = malloc(sizeof (struct VoidNode));
	assert(n != NULL);

    n->data = malloc(size);
    n->size = size;

	n->next = NULL;

    memcpy(n->data, data, size);

	return n;
}

void VoidListInsert(VoidList L, void *data, int size) {
	struct VoidNode *n;

	assert(L != NULL);
	n = newListNode(data, size);
	if (L->first == NULL)
		L->first = L->last = n;
	else {
		L->last->next = n;
		L->last = n;
	}
	L->size++;
}

void VoidListDelete(VoidList L, void *data) {
	struct VoidNode *curr, *prev;

	assert(L != NULL);

	// find where v occurs in list
	prev = NULL; curr = L->first;
    while (curr != NULL && data != curr->data) {
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

struct VoidNode *VoidListGet(VoidList L, int n) {
	int i = 0;
	struct VoidNode *curr = L->first;

	for (;i < n && curr != NULL; i++) curr = curr->next;
	return curr;
}

struct VoidNode *VoidListIterate(VoidList L, int *it) {
    if (L->size == 0) return NULL;

	*it = *it + 1;
    if (*it >= L->size) {
        *it = VOIDLIST_ITERTOREND;
        return NULL;
    }

    struct VoidNode *node = VoidListGet(L, *it);
    assert(node != NULL);

    return node;
}

int VoidListLength(VoidList L) {
	assert(L != NULL);
	return L->size;
}