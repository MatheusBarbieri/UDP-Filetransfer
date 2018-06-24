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
    if (argc < 4) {
        std::cout << "Usage:\n\t./udpClient <username> <host> <port>" << std::endl;
        return 0;
    }
    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);

    UDPClient* client = new UDPClient(username, port, host);
    FILE * file = fopen("doggo.jpeg", "r");

    client->connect();
    client->sendFile(file);
    std::cout << "Mandou a file!\n" << std::endl;

    fclose(file);
    close(client->getSocketDesc());

    return 0;
}
