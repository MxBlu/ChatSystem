/*
 * TCP socket initialisation code acquired from COMP1917 Ass2B Mandelbrot,
 *   base code by Tim Lambert 02/04/12 from his bmpServer.c
 *
*/
#include <stdio.h>

#include "Socket.h"

void Sleep(int sleepMs) {
    usleep(sleepMs * 1000);
}

Socket createSocket(unsigned short port) {
    Socket sockfd = socket (AF_INET, SOCK_STREAM, 0); // Create socket
    if (sockfd <= 0) return 0;

    const int optionValue = 1;
    socklen_t size = sizeof(socklen_t);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optionValue, size); // Allow port to be reused without delay

    struct sockaddr_in server; // Set up local socket info
    memset (&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0) { // Bind socket to port
        closeSocket(sockfd);
        return 0;
    }

    return sockfd;
}

Socket connectTo(char *address, char *port) {
    Socket clientfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (clientfd <= 0) return 0;

    struct sockaddr_in client; // Set client info
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = 0;

    if (bind(clientfd, (const struct sockaddr*) &client, sizeof(client)) <  0) { // Bind socket to any available port
        closeSocket(clientfd);
        return 0;
    }

    struct addrinfo hints, *serveraddr; // Set server hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(address, port, &hints, &serveraddr) != 0) { // Get address struct for server
        closeSocket(clientfd);
        return 0;
    }

    if (connect(clientfd, serveraddr->ai_addr, serveraddr->ai_addrlen)) { // Connect to server
        closeSocket(clientfd);
        freeaddrinfo(serveraddr);
        return 0;
    }

    freeaddrinfo(serveraddr);
    return clientfd;
}

void closeSocket(Socket sockfd) {
    if (sockfd <= 0)
        return;

    close(sockfd);
}

Socket getClientSocket(Socket serverSocket, in_addr_t *address) {
    listen (serverSocket, MAX_SERVERBACKLOG); // Check for any new clients

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    memset(&client, 0, clientLen);
    Socket sockfd = accept(serverSocket, (struct sockaddr *) &client, &clientLen); // Get client socket
    *address = client.sin_addr.s_addr;

    if (sockfd <= 0) return 0;

    return sockfd;
}

void setNonblocking(Socket sockfd) {
    fcntl(sockfd, F_SETFL, O_NONBLOCK, 1);
}

void setBlocking(Socket sockfd, int timeout) {
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK, 0) == -1) {
        return;
    }

    struct timeval tv; // Set timeout
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout - tv.tv_sec*1000)*1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}