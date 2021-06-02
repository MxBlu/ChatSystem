// LinkedList.c, list of username-passwords
// Initially written by John Shepherd, July 2008
// Modified by Jumail Mundekakt, March 2017

#ifndef INTLIST_H
#define INTLIST_H

#include <stdio.h>

#define LIST_AUTHSUCCESS    1
#define LIST_AUTHFAILED     0

typedef struct ListRep *List;
List newList();
void freeList(List);
List getList(FILE *);
void ListInsert(List, char *, char *);
void ListDelete(List, char *);
int ListCheck(List L, char *user, char *pass);
int ListLength(List);
List ListCopy(List);

#endif
