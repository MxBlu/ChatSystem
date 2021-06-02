// VoidList.c, list of void pointer
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#ifndef VOIDLIST_H
#define VOIDLIST_H

#include <stdio.h>

#define VOIDLIST_ITERTORINIT -1
#define VOIDLIST_ITERTOREND -1

typedef struct VoidRep *VoidList;

struct VoidNode {
    void *data;
    int size;
    struct VoidNode *next;
};

VoidList newVoidList();
void freeVoidList(VoidList L);

void VoidListInsert(VoidList L, void *data, int size);
void VoidListDelete(VoidList L, void *data);

struct VoidNode *VoidListGet(VoidList L, int n);
struct VoidNode *VoidListIterate(VoidList L, int *it);
int VoidListLength(VoidList L);

#endif
