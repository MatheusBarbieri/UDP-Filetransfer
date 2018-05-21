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
    if (argc < 3) {
        std::cout << "Usage:\n./udpClient <username> <host> <port>" << std::endl
        return 0;
    }

    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);
    int n;
    int port;
    socklen_t clilen;
    char buf[256];

    UDPServer* server = new UDPServer(port);

    clilen = sizeof(struct sockaddr_in);

    while (true) {
        /* receive from socket */
        n = recvfrom(sockfd, buf, 256, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (n < 0)
        printf("ERROR on recvfrom");
        std::cout << "waht" << std::endl;
        printf("Received a datagram: %s\n", buf);

        /* send to socket */
        n = sendto(sockfd, "Got your message\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
        if (n  < 0)
        printf("ERROR on sendto");
    }

    close(sockfd);
    return 0;
}
