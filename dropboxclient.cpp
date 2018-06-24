#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

#include "client.hpp"

int main(int argc, char **argv){
    if (argc < 4) {
        std::cout << "Usage:\n\t./udpClient <username> <host> <port>" << std::endl;
        return 0;
    }
    std::string username = argv[1], host = argv[2];
    int port = atoi(argv[3]);

    UDPClient* client = new UDPClient(username, port, host);
    client->connect();

    std::string clientFolder = setUpClientFolder(username);

    //ioLoop

    FILE * file = fopen("doggo.jpeg", "r");
    client->sendFile(file);
    std::cout << "Mandou a file!\n" << std::endl;

    fclose(file);
    close(client->getSocketDesc());

    return 0;
}
