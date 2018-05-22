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


///////////////////////////////////////////////////////////
///////////////// NON CLASS FUNCTIONS /////////////////////
///////////////////////////////////////////////////////////

void zerosDatagram (Datagram* dg){
    memset(dg, 0, DGRAMSIZE);
}

///////////////////////////////////////////////////////////
//////////////// END NON CLASS FUNCTIONS //////////////////
///////////////////////////////////////////////////////////
////////////////////// CONSTRUCTORS ///////////////////////
///////////////////////////////////////////////////////////


UDPClient::UDPClient(std::string username, int port, std::string ip){
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
    this->username = username;
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

int UDPConnection::sendDatagram(Datagram dg) {
    socklen_t socketSize = sizeof(socketAddr);
    int status = 0;

    Datagram ack;
    zerosDatagram(&ack);

    //0.3 seconds timeout on socket recv
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 300000;
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
                     (void*) &ack,
                     DGRAMSIZE,
                     0,
                     (struct sockaddr *) &socketAddrFrom,
                     &socketSize
            );
            if (ack.type == ACK && ack.seqNumber == dg.seqNumber){
                break;
            }
        } else{
            std::cout << "[Error] Sending datagram." << std::endl;
        }
    }
    return status;
}

int UDPConnection::recDatagram(){

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
        Datagram* received = (Datagram*) &recvbuffer;
        Datagram ack;
        zerosDatagram(&ack);

        ack.type = ACK;
        ack.seqNumber = received->seqNumber;

        socklen_t socketFromSize = sizeof(socketAddrFrom);
        sendto(socketDesc,
               (void*) &ack,
               DGRAMSIZE,
               0,
               (struct sockaddr *) &socketAddrFrom,
               socketFromSize
        );
    }
    return status;
}

void UDPServer::_bind(){
    if (bind(socketDesc, (struct sockaddr *) &socketAddr, sizeof(struct sockaddr)) < 0){
        std::cout << "[Error] could not bind the given socket. Is this user already connected?" << std::endl;
        exit(0);
    }
}

int UDPClient::connect(){
    Datagram connectDatagram;
    zerosDatagram(&connectDatagram);
    connectDatagram.type = 1;
    connectDatagram.seqNumber = -1;
    username.copy(connectDatagram.data, DATASIZE);
    int sent = sendDatagram(connectDatagram);
    return sent;
}

int UDPServer::connect(){
    int received = recDatagram();
    username = getRecvbuffer()->data;
    std::cout << "Usuário " << username << " logado!" << std::endl;
    return received;
}

Datagram* UDPConnection::getRecvbuffer(){
    return (Datagram*) &recvbuffer;
}

bool UDPConnection::isConnected(){
    return this->connected;
}
