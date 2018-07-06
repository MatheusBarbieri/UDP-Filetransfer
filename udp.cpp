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
#include "util.hpp"


///////////////////////////////////////////////////////////
///////////////// NON CLASS FUNCTIONS /////////////////////
///////////////////////////////////////////////////////////

int getFileSize(FILE *file);
void zerosDatagram (Datagram* dg);

///////////////////////////////////////////////////////////
//////////////// END NON CLASS FUNCTIONS //////////////////
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
/////////////////////// UDPServer /////////////////////////
///////////////////////////////////////////////////////////

UDPServer::UDPServer(){}

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

    socketDesc = socketd;
    socketAddrLocal = sockaddr;
    socketAddrRemote = sockaddr;
}

UDPServer::~UDPServer(void){
    ::close(this->socketDesc);
}

void UDPServer::_bind(){
    if (bind(socketDesc, (struct sockaddr *) &  socketAddrLocal, sizeof(struct sockaddr)) < 0){
        std::cout << "[Error] could not bind the given socket. Is adress already bound?" << std::endl;
        exit(0);
    }
}

udpconnection_ptr UDPServer::accept() {
    recDatagram();
    vlog("Server recebeu connect: dg1");
    int port = generatePort();
    Datagram* recvDg = getRecvbuffer();
    Datagram acceptDatagram;
    zerosDatagram(&acceptDatagram);
    acceptDatagram.type = ACCEPT;
    acceptDatagram.seqNumber = port;

    vlog("Server vai mandar o accept dg2");
    int sent = sendDatagram(acceptDatagram);
    vlog("Server mandou o accept");

    udpconnection_ptr udpconnection(new UDPConnection(port, socketAddrRemote, socketAddrLocal));

    socketAddrRemote.sin_addr.s_addr = INADDR_ANY;
    bzero(&(socketAddrRemote.sin_zero), 8);

    vlog("Server vai retornar conn");
    return udpconnection;
}

///////////////////////////////////////////////////////////
/////////////////////// UDPClient /////////////////////////
///////////////////////////////////////////////////////////

UDPClient::UDPClient(){}

UDPClient::UDPClient(int port, std::string ip) {
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
    socketAddrRemote = sockaddr;
}

UDPClient::~UDPClient(void){
    ::close(this->socketDesc);
}

int UDPClient::connect(){
    Datagram connectDatagram;
    zerosDatagram(&connectDatagram);
    connectDatagram.type = CONNECT;
    connectDatagram.seqNumber = -1;
    vlog("Vai enviar connect: dg1");
    int sent = sendDatagram(connectDatagram);
    vlog("Enviou connect");
    return sent;
}

int UDPClient::waitResponse(){
    vlog("Cliente espera resposta em waitResponse: dg2");
    recDatagram();
    if (getRecvbuffer()->type == ACCEPT){
        Datagram* received = getRecvbuffer();
        std::cerr << "port = " << received->seqNumber << '\n';
        std::cerr << "htons(port) = " << htons(received->seqNumber) << '\n';

        socketAddrRemote.sin_port = htons(received->seqNumber);
        socketAddrLocal.sin_port = htons(received->seqNumber);
        bzero(&(socketAddrRemote.sin_zero), 8);
        bzero(&(socketAddrLocal.sin_zero), 8);

        ::close(this->socketDesc);
        int socketd = 0;
        if ((socketd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
            std::cout << "[Error] Could not open socket." << std::endl;
        }

        socketDesc = socketd;
        return ACCEPT;

    }
    return REJECT;
}

struct sockaddr_in* UDPSocket::getAddr(){
    return &socketAddrLocal;
}

int UDPSocket::getSocketDesc(){
    return socketDesc;
}

///////////////////////////////////////////////////////////
///////////////////// UDPConnection ///////////////////////
///////////////////////////////////////////////////////////

UDPConnection::UDPConnection(){}

UDPConnection::UDPConnection(int port, sockaddr_in socketAddrRemote, sockaddr_in socketAddrLocal) {
    this->socketAddrLocal = socketAddrLocal;
    this->socketAddrRemote = socketAddrRemote;
    this->socketAddrRemote.sin_port = htons(port);
    this->socketAddrLocal.sin_port = htons(port);
    bzero(&(this->socketAddrLocal.sin_zero), 8);
    bzero(&(this->socketAddrRemote.sin_zero), 8);

    std::cerr << "port = " << port << '\n';
    std::cerr << "htons(port) = " << htons(port) << '\n';

    if ((this->socketDesc = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        std::cout << "[Error] Could not open a new socket for connection." << std::endl;
    }

    if (bind(socketDesc, (struct sockaddr *) &  this->socketAddrLocal, sizeof(struct sockaddr)) < 0){
        std::cout << "[Error] could not bind the given socket. Is adress already bound?" << std::endl;
    }
}



struct sockaddr_in* UDPConnection::getAddrRemote(){
    return &socketAddrRemote;
}

void UDPConnection::close() {

}

int UDPConnection::sendDatagram(Datagram &dg) {
    socklen_t socketSize = sizeof(socketAddrRemote);
    int status = 0;

    Datagram ack;
    Datagram senddg;
    zerosDatagram(&senddg);
    senddg.type = htonl(dg.type);
    senddg.seqNumber = htonl(dg.seqNumber);
    senddg.size = htonl(dg.size);
    memcpy(senddg.data, dg.data, DATASIZE);
    zerosDatagram(&ack);

    //0.3 seconds timeout on socket recv
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 300000;
    setsockopt(this->getSocketDesc(), SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    while(true){
        status = sendto(getSocketDesc(),
                        (char*) &senddg,
                        DGRAMSIZE,
                        0,
                        (struct sockaddr *) &socketAddrRemote,
                        socketSize
        );
        if (status >= 0){
            socklen_t socketSize = sizeof(socketAddrRemote);
            recvfrom(getSocketDesc(),
                     (void*) &ack,
                     DGRAMSIZE,
                     0,
                     (struct sockaddr *) &socketAddrRemote,
                     &socketSize
            );
            if (ntohl(ack.type) == ACK && ack.seqNumber == dg.seqNumber){
                break;
            }
        } else{
            std::cout << "[Error] Sending datagram." << std::endl;
        }
    }
    return status;
}

int UDPConnection::sendDatagramMaxTries(Datagram &dg, int maxTries) {
    socklen_t socketSize = sizeof(socketAddrRemote);
    int status = 0, tries = 0;

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
                        (struct sockaddr *) &socketAddrRemote,
                        socketSize
        );
        if (status >= 0){
            socklen_t socketSize = sizeof(socketAddrRemote);
            recvfrom(getSocketDesc(),
                     (void*) &ack,
                     DGRAMSIZE,
                     0,
                     (struct sockaddr *) &socketAddrRemote,
                     &socketSize
            );
            if (ack.type == ACK && ack.seqNumber == dg.seqNumber){
                break;
            }
            tries++;
            if (tries > maxTries){
                return TIMEOUT;
            }
        } else {
            std::cout << "[Error] Sending datagram." << std::endl;
        }
    }
    return status;
}

int UDPConnection::recDatagram(){

    socklen_t socketSize = sizeof(socketAddrRemote);
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
                          (struct sockaddr *) &socketAddrRemote,
                          &socketSize
    );

    if (status < 0){
        std::cout << "[Error] Could not receive Datagram." << std::endl;
    } else {
        Datagram* received = (Datagram*) &recvbuffer;
        Datagram ack;
        zerosDatagram(&ack);

        ack.type = htonl(ACK);
        ack.seqNumber = htonl(received->seqNumber);

        socklen_t socketFromSize = sizeof(socketAddrRemote);
        sendto(socketDesc,
               (void*) &ack,
               DGRAMSIZE,
               0,
               (struct sockaddr *) &socketAddrRemote,
               socketFromSize
        );
    }
    Datagram* received = (Datagram*) &recvbuffer;
    received->type = ntohl(received->type);
    received->seqNumber = ntohl(received->seqNumber);
    received->size = ntohl(received->size);
    return status;
}

int UDPConnection::recDatagramTimeOut(int timeOut){
    socklen_t socketSize = sizeof(socketAddrRemote);
    struct timeval read_timeout;
    read_timeout.tv_usec = 0;
    read_timeout.tv_sec = timeOut;

    setsockopt(socketDesc,
               SOL_SOCKET, SO_RCVTIMEO,
               &read_timeout,
               sizeof(read_timeout)
    );

    int status = recvfrom(getSocketDesc(),
                          &recvbuffer,
                          DGRAMSIZE,
                          0,
                          (struct sockaddr *) &socketAddrRemote,
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

        socklen_t socketFromSize = sizeof(socketAddrRemote);
        sendto(socketDesc,
               (void*) &ack,
               DGRAMSIZE,
               0,
               (struct sockaddr *) &socketAddrRemote,
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

std::string UDPConnection::receiveString(){
    char* c_message = NULL;
    c_message = receiveMessage();
    std::string message = std::string(c_message);
    free(c_message);
    return message;
}

int UDPConnection::sendMessage(char* message, int length){
    // std::cerr << "<sendMessage Start>" << '\n';
    int numDatagrams = (length/DATASIZE)+1;
    if ((length%DATASIZE) == 0) {
        numDatagrams -= 1;
    }
    Datagram messageDatagram;
    zerosDatagram(&messageDatagram);
    messageDatagram.type = MESSAGE;
    messageDatagram.seqNumber = 0;
    messageDatagram.size = length;

    int status = sendDatagram(messageDatagram);
    if(status > 0){
        int seqNumber;
        for(seqNumber=0; seqNumber<numDatagrams; seqNumber++){
            zerosDatagram(&messageDatagram);
            messageDatagram.type = DATAGRAM;
            messageDatagram.seqNumber = seqNumber;
            messageDatagram.size = numDatagrams;

            memcpy((void *)  messageDatagram.data,
                   (void *) (message + DATASIZE*seqNumber),
                   DATASIZE
            );
            sendDatagram(messageDatagram);
        }
    }
    // std::cerr << "<sendMessage End>" << '\n';
    return status;
}

long UDPConnection::getRecvMessageSize(){
    return recvMessageSize;
}

char* UDPConnection::receiveMessage(){
    recDatagram();
    Datagram* received = getRecvbuffer();
    if (received->type != MESSAGE){
        std::cout << "[Error] Expected a message, got a non-message." << std::endl;
        return NULL;
    }
    uint32_t receivedMessageSize = received->size;
    int numDatagrams = (receivedMessageSize/DATASIZE)+1;
    if (receivedMessageSize%DATASIZE == 0){
        numDatagrams--;
    }
    char* receivedMessage = (char*) malloc(numDatagrams * DATASIZE);

    int seqNumber;
    for(seqNumber=0; seqNumber<numDatagrams; seqNumber++){
        recDatagram();
        memcpy((receivedMessage + DATASIZE * seqNumber),
               received->data,
               DATASIZE
        );
    }
    recvMessageSize = receivedMessageSize;
    return receivedMessage;
}

int UDPConnection::sendFile(FILE* file){
    int fileSize = getFileSize(file);
    int lastDatagramSize = fileSize%DATASIZE;
    Datagram fileDatagram;
    zerosDatagram(&fileDatagram);
    fileDatagram.type = DATAFILE;
    fileDatagram.seqNumber = 0;
    fileDatagram.size = lastDatagramSize;
    int status = sendDatagram(fileDatagram);
    if(status < 0){
        return -1;
        std::cout << "[Error] Could not send sendFile request." << std::endl;
    }

    char buffer[DATASIZE] = {0};
    int seqNumber = 0;

    int reading = 1;
    while (reading == 1) {
        reading = fread(buffer, DATASIZE, 1, file);
        zerosDatagram(&fileDatagram);
        if (reading == 1){
            fileDatagram.type = DATAGRAM;
            fileDatagram.seqNumber = seqNumber++;
            fileDatagram.size = 0;
            memcpy((void *) fileDatagram.data,
                   (void *) buffer,
                   DATASIZE
            );
        }
        else{
            fileDatagram.type = ENDFILE;
            fileDatagram.seqNumber = seqNumber++;
            fileDatagram.size = 0;
            memcpy((void *) fileDatagram.data,
                   (void *) buffer,
                   lastDatagramSize
            );
        }

        sendDatagram(fileDatagram);
        memset(&buffer, 0, DATASIZE);
    }

    return status;
}

int UDPConnection::receiveFile(FILE* file){
    int receivedFile = recDatagram();
    Datagram* received = getRecvbuffer();
    if (received->type != DATAFILE){
        std::cout << "[Error] Expected a file, got a non-file." << std::endl;
        return -1;
    }

    int lastDatagramSize = received->size;
    int size = DATASIZE;

    int seqNumber = 0;
    while(received->type != ENDFILE){
        recDatagram();
        if(seqNumber == received->seqNumber){
            if (received->type == ENDFILE){
                size = lastDatagramSize;
            }
            fwrite(received->data, size, 1, file);
            seqNumber++;
        }
    }
    return receivedFile;
}

Datagram* UDPConnection::getRecvbuffer(){
    return (Datagram*) &recvbuffer;
}

bool UDPConnection::isConnected(){
    return this->connected;
}

int getFileSize(FILE *file) {
    fseek(file, 0L, SEEK_END);
    int fileSize = ftell(file);
    rewind(file);
    return fileSize;
}

void zerosDatagram (Datagram* dg){
    memset(dg, '\0', DGRAMSIZE);
}
