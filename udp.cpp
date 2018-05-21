#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>

#include "udp.hpp"

UDPClient::UDPClient(int port, std::string ip){
    int socketd = 0;
    if ((socketd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        std::cout << "[Error] Could not open socket." << std::endl;
    }

    struct hostent* server_host = gethostbyname(ip.c_str());
    if (server_host == NULL) {
        std::cout << "[Error] Host not found." << std::endl;
        exit(0);
    }

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr = *((struct in_addr *)server_host->h_addr);
    bzero(&(sockaddr.sin_zero), 8);

    socketDesc = socketd;
    socketAddr = sockaddr;
}

UDPClient::~UDPClient(void){
    close(this->socketDesc);
}

UDPServer::UDPServer(int port){
    int socketd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketd == -1){
        std::cout << "[Error] Could not open socket." << std::endl;
    }

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(sockaddr.sin_zero), 8);

    UDPServer::socketDesc = socketd;
    UDPServer::socketAddr = sockaddr;
}


UDPServer::~UDPServer(void){
    close(this->socketDesc);
}

///////////////////////////////////////////////////////////
//////////////////// END CONSTRUCTORS /////////////////////
///////////////////////////////////////////////////////////

struct sockaddr_in* UDPSocket::getAddr(){
    return &socketAddr;
}

int UDPSocket::getSocketDesc(){
    return socketDesc;
}

struct sockaddr_in* UDPConnection::getAddrFrom(){
    return &socketAddrFrom;
}

// n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *) &cli_addr, &clilen);

void UDPConnection::sendDatagram(Datagram dg) {
    socklen_t socketSize = sizeof(socketAddr);
    char ack[3] = {0,0,0};
    int status = 0;

    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 300000; // 0.3 seconds
    setsockopt(this->getSocketDesc(), SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    while(true){
        status = sendto(getSocketDesc(),
        (char*) &dg,
        DGRAMSIZE,
        0,
        (struct sockaddr *) &socketAddr,
        socketSize
    );
    if (status >= 0){
        socklen_t socketSize = sizeof(socketAddrFrom);
        recvfrom(getSocketDesc(),
        ack,
        3,
        0,
        (struct sockaddr *) &socketAddrFrom,
        &socketSize
    );
    if (strcmp(ack, "ack")){

    }
}
}
}

void UDPConnection::recDatagram(){
    socklen_t socketSize = sizeof(socketAddr);
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 0;
    setsockopt(socketDesc,
        SOL_SOCKET, SO_RCVTIMEO,
        &read_timeout,
        sizeof(read_timeout)
    );

    int status = recvfrom(getSocketDesc(),
    &recvbuffer,
    DGRAMSIZE,
    0,
    (struct sockaddr *) &socketAddrFrom,
    &socketSize
);
if (status < 0){
    std::cout << "[Erro] Could not receive Datagram." << std::endl;
} else {
    sendto(socketDesc,
        "ack",
        3,
        0,
        (struct sockaddr *) &socketAddrFrom,
        socketSize
    );
}
}

void UDPServer::_bind(){
    if (bind(socketDesc, (struct sockaddr *) &socketAddr, sizeof(struct sockaddr)) < 0){
        std::cout << "[Error] could not bind the given socket to the given address." << std::endl;
        exit(0);
    }
}

void UDPServer::bindAddr(struct sockaddr_in boundAddr){
    if (bind(socketDesc, (struct sockaddr *) &boundAddr, sizeof(struct sockaddr)) < 0){
        std::cout << "[Error] could not bind the given socket to the given address." << std::endl;
        exit(0);
    }
}

int UDPClient::connect(){
    socklen_t socketSize = sizeof(socketAddr);
    int status = sendto(this->getSocketDesc(),
    &sendbuffer,
    DGRAMSIZE,
    0,
    (struct sockaddr *) &socketAddr,
    socketSize
);
if (status >= 0){
    this->connected = true;
    return 1;
} else {
    return 0;
}
}

int UDPServer::connect(){
    socklen_t socketSize = sizeof(socketAddrFrom);
    int status = recvfrom(socketDesc,
        &recvbuffer,
        DGRAMSIZE,
        0,
        (struct sockaddr *) &socketAddrFrom,
        &socketSize
    );
    return status;
}

bool UDPConnection::isConnected(){
    return this->connected;
}
