#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <cstring>

#include "udp.hpp"

int main(int argc, char *argv[]){
    if (argc < 2) {
        std::cout << "Usage:\n\t./udpServer <port>" << std::endl;
        return 0;
    }

    int port = atoi(argv[1]);

    UDPServer* server = new UDPServer(port);
    server->_bind();
    server->connect();

    FILE * file = fopen("cat.jpeg", "w+");
    server->receiveFile(file);
    fclose(file);

    close(server->getSocketDesc());
    return 0;
}
