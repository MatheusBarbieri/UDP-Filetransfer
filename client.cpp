#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include "udp.hpp"

#define PORT 4000

int main(int argc, char **argv){
    if (argc < 3) {
        std::cout << "Usage:\n./udpClient <username> <host> <port>" << std::endl
        return 0;
    }

    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);
    char buffer[256];

    UDPClient* client = new UDPClient(port, host);

    printf("Enter the message:");
    bzero(buffer, 256);
    fgets(buffer, 256, stdin);

    int n = sendto(client->getSocketDesc(), buffer, strlen(buffer), 0, (const struct sockaddr *) client->getAddr(), sizeof(struct sockaddr_in));
    if (n < 0)
    printf("ERROR sendto");

    socklen_t length = sizeof(struct sockaddr_in);
    n = recfrom(client->getSocketDesc(), buffer, 256, 0, (struct sockaddr *) client->getAddrFrom(), &length);
    if (n < 0)
    printf("ERROR recvfrom");

    printf("Got an ack: %s\n", buffer);

    close(client->getSocketDesc());
    return 0;
}
