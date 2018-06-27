#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <cstring>

#include "server.hpp"

int main(int argc, char *argv[]){
    if (argc < 2) {
        std::cout << "Usage:\n\t./udpServer <port>" << std::endl;
        return 0;
    }

    int port = atoi(argv[1]);

    Server server(port);
    //
    // while(true){
    //     UDPServer server(port);
    //     server._bind();
    //     server.connect();
    // }

    // FILE * file = fopen("cat.jpeg", "w+");
    // server.receiveFile(file);
    // fclose(file);

    // close(server.getSocketDesc());
    return 0;
}
