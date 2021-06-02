
CC = gcc

BINS = client server
COMMON = Socket.o ChatProtocol.o

CLIENT = 
SERVER = ClientList.o UserList.o BlockedList.o MessageList.o BlockedAddrList.o VoidList.o

all : $(BINS)

client : client.o $(COMMON) $(CLIENT)
server : server.o  $(COMMON) $(SERVER)

client.o : client.c
server.o : server.c

Socket.o : Socket.c Socket.h
ChatProtocol.o : VoidList.o VoidList.h ChatProtocol.c ChatProtocol.h
BlockedList.o : BlockedList.c UserList.h
UserList.o : VoidList.o VoidList.h UserList.c UserList.h
MessageList.o : VoidList.o VoidList.h MessageList.c MessageList.h
BlockedAddrList.o : VoidList.o VoidList.h BlockedAddrList.c BlockedAddrList.h
VoidList.o : VoidList.c VoidList.h

clean : 
	rm -f $(BINS) *.o
