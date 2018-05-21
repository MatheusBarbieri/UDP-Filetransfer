#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>

#define DGRAMSIZE 512

typedef struct defaultDatagram {
    int type;
    int sequence;
    char data[DGRAMSIZE - 2*sizeof(int)];
} Datagram;

typedef struct startDatagram {
    int type;
    std::string userName;
} StartDatagram;

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
public:
    void sendDatagram(Datagram dg);
    void recDatagram();
    char sendbuffer[DGRAMSIZE];
    char recvbuffer[DGRAMSIZE];
    struct sockaddr_in* getAddrFrom();
    bool isConnected();
};

class UDPClient: public UDPConnection {
public:
    UDPClient(int port, std::string ip);
    ~UDPClient();

    int connect();

    void send(char* buffer, size_t length);
    void recieve(char* buffer, size_t length);
};

class UDPServer: public UDPConnection {
public:
    UDPServer(int port);
    ~UDPServer();

    int connect();
    void _bind();
    void bindAddr(struct sockaddr_in boundAddr);

    void send(char* buffer, size_t length);
    void recieve(char* buffer, size_t length);
};
