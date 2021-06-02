//
// Created by Jumail on 18/03/2017.
//

#ifndef HTTP_SERVER_SOCKET_H
#define HTTP_SERVER_SOCKET_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

typedef int Socket;

void Sleep(int sleepMs);

#define MAX_SERVERBACKLOG 20

Socket createSocket (unsigned short port);
Socket connectTo (char *address, char *port);
void closeSocket(Socket sockfd);

Socket getClientSocket(Socket serverSocket, in_addr_t *address);

void setNonblocking(Socket sockfd);
void setBlocking(Socket sockfd, int timeout);

#endif //HTTP_SERVER_SOCKET_H
