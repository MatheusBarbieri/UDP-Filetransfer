#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>

#define DGRAMSIZE 512
#define DATASIZE (DGRAMSIZE-3*sizeof(int))

#define CONNECT 1
#define ACK 2
#define MESSAGE 3
#define DATAGRAM 4
#define DATAFILE 5
#define ENDFILE 6


typedef struct datagram {
    int type;
    int seqNumber;
    int size;
    char data[DATASIZE];
} Datagram;

void zerosDatagram (Datagram* dg);

class UDPSocket {
protected:
    struct sockaddr_in socketAddr;
    int socketDesc;
public:
    struct sockaddr_in* getAddr();
    int getSocketDesc();
};

class UDPConnection: public UDPSocket {
protected:
    bool connected;
    struct sockaddr_in socketAddrFrom;
    std::string username;

public:
    int sendDatagram(Datagram dg);
    int recDatagram();
    char sendbuffer[DGRAMSIZE];
    char recvbuffer[DGRAMSIZE];

    Datagram* getRecvbuffer();
    int sendString(std::string str);
    int sendMessage(char* buffer, int length);
    int sendFile(FILE* file);
    char* receiveMessage();
    int receiveFile(FILE* file);

    struct sockaddr_in* getAddrFrom();
    bool isConnected();
};

class UDPClient: public UDPConnection {
public:
    UDPClient(std::string username, int port, std::string ip);
    ~UDPClient();
    int connect();
};

class UDPServer: public UDPConnection {
public:
    UDPServer(int port);
    ~UDPServer();
    int connect();
    void _bind();
};
