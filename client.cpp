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

int main(int argc, char **argv){
    if (argc < 3) {
        std::cout << "Usage:\n\t./udpClient <username> <host> <port>" << std::endl;
        return 0;
    }

    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);

    UDPClient* client = new UDPClient(username, port, host);

    client->connect();

    Datagram datinha;
    printf("Enter the message:\n");
    bzero(datinha.data, DATASIZE);
	fgets(datinha.data, DATASIZE, stdin);
    datinha.type = CONNECT;
    datinha.seqNumber = 3;

    client->sendDatagram(datinha);
    std::cout << "NICE!\n" << std::endl;

    close(client->getSocketDesc());
    return 0;
}
