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

#define DGRAMSIZE 512
#define DATASIZE DGRAMSIZE-2*sizeof(int)

#define CONNECT 1
#define ACK 2

typedef struct datagram {
    int type;
    int seqNumber;
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

    struct sockaddr_in* getAddrFrom();
    bool isConnected();
};

class UDPClient: public UDPConnection {
public:
    UDPClient(std::string username, int port, std::string ip);
    ~UDPClient();

    int connect();

    void send(char* buffer, size_t length);
    void sendFile(FILE* file, size_t length);
    void recieve(char* buffer, size_t length);
};

class UDPServer: public UDPConnection {
public:
    UDPServer(int    port);
    ~UDPServer();

    int connect();

    void _bind();

    void send(char* buffer, size_t length);
    void sendFile(char* buffer, size_t length);
    void recieve(char* buffer, size_t length);
};
