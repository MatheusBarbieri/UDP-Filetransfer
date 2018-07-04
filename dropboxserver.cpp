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

    server_ptr server(new Server);

    udpserver_ptr udpserver(new UDPServer(port));
    udpserver->_bind();
    while(true){
        udpserver->connect(); //wait for new users
        udpserver_ptr udpsession(new UDPServer(udpserver->socketAddrFrom, port)); //create a UDPServer using new user from addr
        auto it = server->getUsers().find(udpsession->getUsername()); //Look if user exists already
        if (it != server->getUsers().end()){
            user_ptr user(it->second);
            if(user->canConnect()){
                usersession_ptr session(new UserSession(udpsession, user));
                session->thread = std::thread(&UserSession::runSession, session.get());
                user->addSession(session);
                udpsession->accept();
            } else {
                std::cout << "User limit reached!" << std::endl;
                udpsession->reject();
            }
        } else {
            std::cout << "não achou users" << std::endl;
            udpserver->accept();
            std::cout << "Acceptou" << std::endl;

            user_ptr newUser(new User(udpserver->getUsername()));

            usersession_ptr session(new UserSession(udpserver, newUser));
            session->thread = std::thread(&UserSession::runSession, session.get());
            newUser->addSession(session);
            server->getUsers()[udpserver->getUsername()] = std::move(newUser); //Bota o user no map
        }
    }

    std::cout << "Gracefully exiting?" << std::endl;
    return 0;
}
