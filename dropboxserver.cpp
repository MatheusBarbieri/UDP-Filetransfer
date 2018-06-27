#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <cstring>

#include "server.hpp"
#include "udp.hpp"

int main(int argc, char *argv[]){
    if (argc < 2) {
        std::cout << "Usage:\n\t./udpServer <port>" << std::endl;
        return 0;
    }

    int port = atoi(argv[1]);

    Server server;

    while(true){
        UDPServer udpserver(port);
        udpserver._bind();
        udpserver.connect(); //wait for new user
        User *user;
        auto it = server.getUsers().find(udpserver.getUsername());
        if (it != server.getUsers().end()){
            user = &it->second;
            if(user->canConnect()){
                UserSession session(udpserver, user);
            }
        } else {
            User newUser(udpserver.getUsername());
            UserSession session(udpserver, user);
        }



    }

    // FILE * file = fopen("cat.jpeg", "w+");
    // server.receiveFile(file);
    // fclose(file);

    return 0;
}
