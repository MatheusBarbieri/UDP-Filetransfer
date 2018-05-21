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
        fprintf(stderr,"[Error] Could not open socket.\n");
  }

  struct hostent* server_host = gethostbyname(ip.c_str());
  if (server_host == NULL) {
        fprintf(stderr,"[Error] Host not found.\n");
        exit(0);
    }

  struct sockaddr_in sockaddr;
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_port = htons(port);
  sockaddr.sin_addr = *((struct in_addr *)server_host->h_addr);
  bzero(&(sockaddr.sin_zero), 8);

  this->socketDesc = socketd;
  this->socketAddr = sockaddr;
}

UDPClient::~UDPClient(void){
  close(this->socketDesc);
}

UDPServer::UDPServer(int port){
  int socketd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketd == -1){
        fprintf(stderr,"[Error] Could not open socket.\n");
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
  return &this->socketAddr;
}

int UDPSocket::getSocketDesc(){
  return this->socketDesc;
}

struct sockaddr_in* UDPConnection::getAddrFrom(){
  return &this->socketAddrFrom;
}

// n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *) &cli_addr, &clilen);

void UDPConnection::sendDatagram(Datagram dg) {
  char ack[3] = {0,0,0};
  int status = 0;

  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 300000; // 0.3 seconds
  setsockopt(this->getSocketDesc(), SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

  while(true){
    status = sendto(this->getSocketDesc(),
                 (char*) &dg,
                 DGRAMSIZE,
                 0,
                 (const struct sockaddr *) &this->socketAddr,
                 sizeof(struct sockaddr_in)
    );
    if (status >= 0){
      recvfrom(this->getSocketDesc(),
               ack,
               3,
               0,
               (const struct sockaddr *) &this->socketAddrFrom,
               sizeof(struct sockaddr_in)
      );
      if (strcmp(ack, "ack")){

      }
    }
  }
}

void UDPConnection::recDatagram(){
  struct timeval read_timeout;
  read_timeout.tv_sec = 0;
  read_timeout.tv_usec = 0; // 0.3 seconds
  setsockopt(this->getSocketDesc(),
             SOL_SOCKET, SO_RCVTIMEO,
             &read_timeout,
             sizeof read_timeou
  );

  int status = recvfrom(this->getSocketDesc(),
               &recvbuffer,
               DGRAMSIZE,
               0,
               (const struct sockaddr *) &this->socketAddrFrom,
               sizeof(struct sockaddr_in)
  );
  if (status < 0){
    cout << "[Erro] Could not receive Datagram." << endl;
  } else {
    int status = sendto(this->getSocketDesc(),
                 "ack",
                 3,
                 0,
                 (const struct sockaddr *) &this->getAddrFrom,
                 sizeof(struct sockaddr_in)
    );
    break;
  }
}

void UDPServer::bind(){
  if (bind(sockfd, (struct sockaddr *) &UDPServer::socketAddr, sizeof(struct sockaddr)) < 0){
    printf("[Error] could not bind the given socket to the given address.");
    exit(0);
  }
}

void UDPServer::bindAddr(struct sockaddr_in boundAddr){
  if (bind(sockfd, (struct sockaddr *) &boundAddr, sizeof(struct sockaddr)) < 0){
    printf("[Error] could not bind the given socket to the given address.");
    exit(0);
  }
}

int UDPClient::connect(){
  int status = sendto(this->getSocketDesc(),
               &sendbuffer,
               DGRAMSIZE,
               0,
               (const struct sockaddr *) &this->getAddrFrom(),
               sizeof(struct sockaddr_in)
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
               (const struct sockaddr *) &socketAddrFrom,
               &socketSize
  );
  return status;
}

bool UDPConnection::isConnected(){
  return this->connected;
}
