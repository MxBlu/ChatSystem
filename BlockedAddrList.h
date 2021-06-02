//
// Created by mxblue on 17/04/17.
//

#include "Socket.h"

#ifndef ASS1_BLOCKEDADDRLIST_H
#define ASS1_BLOCKEDADDRLIST_H

typedef struct BlockedAddrRep *BlockedAddrList;

BlockedAddrList newBlockedAddrList(int block_duration);
void freeBlockedAddrList(BlockedAddrList L);

void BlockAddr(BlockedAddrList L, in_addr_t clientAddr);
int AddrIsBlocked(BlockedAddrList L, in_addr_t clientAddr);

#endif //ASS1_BLOCKEDADDRLIST_H
