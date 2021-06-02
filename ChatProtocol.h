//
// Created by Jumail on 22/03/2017.
//

#ifndef ASS1_CHATPROTOCOL_H
#define ASS1_CHATPROTOCOL_H

#include "Socket.h"

#define CHAT_HEADERSIZE 9
#define CHAT_PROTOCOLID 0x20bcac79 //CRC32: jumail_chatprotocol

#define CHAT_INTENT_NULL 0

#define CHAT_INTENT_AUTHREQUIRED 'a'
#define CHAT_INTENT_AUTHENTICATE 'b'

#define CHAT_INTENT_SUCCESS 'c'
#define CHAT_INTENT_FAILED 'd'
#define CHAT_INTENT_ALREADY 'e'

#define CHAT_INTENT_PM 'h'
#define CHAT_INTENT_BROADCAST 'k'
#define CHAT_INTENT_BLOCKUSR 'l'
#define CHAT_INTENT_UNBLOCKUSR 'm'
#define CHAT_INTENT_WHOELSE 'i'
#define CHAT_INTENT_WHOELSESINCE 'n'

#define CHAT_INTENT_HEARTBEAT 'j'
#define CHAT_INTENT_DISCONNECT 'f'
#define CHAT_INTENT_CLIENTTIMEDOUT 'g'

#define CHAT_AUTHRETRYCOUNT 3
#define CHAT_CLIENTTIMEOUT 30 * 1000

typedef struct ChatPackRep {
    char intent;
    char *data;
    int size;
} ChatPacket;

int sendChatHeader(Socket clientfd, char intent, int size);
int sendChatData(Socket clientfd, char *data, int size);
int sendChatPacket(Socket clientfd, char intent, char *data, int size);
ChatPacket recvChatPacket(Socket clientfd);
void deleteChatPacket(ChatPacket data);

#endif //ASS1_CHATPROTOCOL_H
