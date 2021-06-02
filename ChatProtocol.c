//
// Created by Jumail on 22/03/2017.
//

#include <string.h>
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <asm/errno.h>
#include <errno.h>

#define CHAT_MSGBUFFERSIZE 1024

#include "ChatProtocol.h"

int sendChatHeader(Socket clientfd, char intent, int size) {
    char header[CHAT_HEADERSIZE];
    int packetSize = 0;

    int protocolID = CHAT_PROTOCOLID;
    memcpy(header, &protocolID, sizeof(protocolID));
    packetSize += sizeof(protocolID);

    memcpy(&header[packetSize], &intent, sizeof(intent));
    packetSize += sizeof(intent);

    memcpy(&header[packetSize], &size, sizeof(size));
    packetSize += sizeof(size);

    assert(packetSize == CHAT_HEADERSIZE);

    return (send(clientfd, header, packetSize, 0) == packetSize);
}

int sendChatData(Socket clientfd, char *data, int size) {
    int bytes = -1;
    int aggBytes = 0;
    while (bytes != 0 && aggBytes < size) {
        bytes = send(clientfd, &data[aggBytes], size - aggBytes, 0);
        if (bytes < 0) break;
        aggBytes += bytes;
    }

    return (aggBytes == size);
}

int sendChatPacket(Socket clientfd, char intent, char *data, int size) {
    char *packet = malloc(size + CHAT_HEADERSIZE);
    int packetSize = 0;

    int protocolID = CHAT_PROTOCOLID;
    memcpy(packet, &protocolID, sizeof(protocolID));
    packetSize += sizeof(protocolID);

    memcpy(&packet[packetSize], &intent, sizeof(intent));
    packetSize += sizeof(intent);

    memcpy(&packet[packetSize], &size, sizeof(size));
    packetSize += sizeof(size);

    memcpy(&packet[packetSize], data, size);
    packetSize += size;

    int bytes = -1;
    int aggBytes = 0;
    while (bytes != 0 && aggBytes < packetSize) {
        bytes = send(clientfd, &packet[aggBytes], packetSize - aggBytes, 0);
        if (bytes < 0) break;
        aggBytes += bytes;
    }

    free(packet);
    return (aggBytes == packetSize);
}


ChatPacket recvChatPacket(Socket clientfd) {
    ChatPacket chatData;
    chatData.intent = CHAT_INTENT_NULL;
    chatData.data = NULL;
    chatData.size = 0;

    char header[CHAT_HEADERSIZE];
    if (recv(clientfd, header, CHAT_HEADERSIZE, 0) != CHAT_HEADERSIZE || *(int *)header != CHAT_PROTOCOLID) {
        return chatData;
    }

    memcpy(&(chatData.intent), &header[4], sizeof(char));
    memcpy(&(chatData.size), &header[5], sizeof(int));

    if (chatData.size == 0) return chatData;

    chatData.data = malloc(chatData.size);

    int bytes = -1;
    int aggBytes = 0;
    char data[CHAT_MSGBUFFERSIZE];

    setBlocking(clientfd, 200);
    while (aggBytes < chatData.size) {
        bytes = recv(clientfd, &chatData.data[aggBytes], chatData.size - aggBytes, 0);
        if (bytes == -1) break;
        aggBytes += bytes;
    }

    if (aggBytes != chatData.size) {
        free(chatData.data);
        chatData.data = NULL;
        return chatData;
    }

    return chatData;
}

void deleteChatPacket(ChatPacket data) {
    if (data.size > 0) free(data.data);
}
