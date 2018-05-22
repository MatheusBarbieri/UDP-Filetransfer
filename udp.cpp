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
        std::cout << "[Error] Could not receive Datagram." << std::endl;
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

int UDPConnection::sendString(std::string str){
    char* message = const_cast<char*>(str.c_str());
    int len = str.size();

    int status = sendMessage(message, len);
    return status;
}

int UDPConnection::sendMessage(char* message, int length){
    int dataSize = DATASIZE;
    int numDatagrams = (length/dataSize)+1;
    Datagram messageDatagram;
    zerosDatagram(&messageDatagram);
    messageDatagram.type = MESSAGE;
    messageDatagram.seqNumber = -3;
    messageDatagram.size = numDatagrams;

    int status = sendDatagram(messageDatagram);
    if(status > 0){
        int seqNumber;
        for(seqNumber=0; seqNumber<numDatagrams; seqNumber++){
            zerosDatagram(&messageDatagram);
            messageDatagram.type = DATAFILE;
            messageDatagram.seqNumber = seqNumber;
            messageDatagram.size = numDatagrams;
            memcpy((void *) &messageDatagram.data,
                   (void *) (message + DATASIZE*seqNumber),
                   DATASIZE
            );
            sendDatagram(messageDatagram);
        }
    }
    return status;
}

char* UDPConnection::receiveMessage(){
    recDatagram();
    Datagram* received = getRecvbuffer();
    if (received->type != MESSAGE){
        std::cout << "[Error] Waiting for message" << std::endl;
        return NULL;
    }

    int numDatagrams = received->size;

    int receivedMessageSize = numDatagrams*DATASIZE+1;
    char* receivedMessage = (char*) calloc(receivedMessageSize, sizeof(char));

    int seqNumber;
    for(seqNumber=0; seqNumber<numDatagrams; seqNumber++){
        recDatagram();
        std::cout << received->data << std::endl;
        memcpy((void *) (receivedMessage + DATASIZE*seqNumber),
               (void *) &received->data,
               DATASIZE
        );
    }
    return receivedMessage;
}

int UDPConnection::sendFile(FILE* file, int length){
    return 0;
}

void UDPConnection::receiveFile(FILE* file){
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
    connectDatagram.type = CONNECT;
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
