#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#include "udp.hpp"

int main(int argc, char *argv[])
{
    if (argc < 1) {
        std::cout << "Usage:\n./udpServer <port>" << std::endl;
        return 0;
    }

    int port = atoi(argv[1]);
    int n;
    socklen_t clilen;
    char buffer[256];

    UDPServer* server = new UDPServer(port);
    server->_bind();

    clilen = sizeof(struct sockaddr_in);
    while (true) {
        /* receive from socket */
        n = recvfrom(server->getSocketDesc(), buffer, 256, 0, (struct sockaddr *) server->getAddrFrom(), &clilen);
        if (n < 0)
            printf("ERROR on recvfrom");
        printf("Received a datagram: %s\n", buffer);

        /* send to socket */
        n = sendto(server->getSocketDesc(), "Got your message\n", 17, 0,(struct sockaddr *) server->getAddrFrom(), sizeof(struct sockaddr));
        if (n  < 0)
            printf("ERROR on sendto");
    }

    close(server->getSocketDesc());
    return 0;
}
