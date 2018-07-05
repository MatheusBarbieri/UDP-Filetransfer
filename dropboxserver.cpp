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
        udpconnection_ptr conn = udpserver->accept(); //wait for new users

        // TODO: get username for login/authentication
        std::string username;

        auto it = server->getUsers().find(username); //Look if user exists already
        if (it == server->getUsers().end()) {
            // create user
            user_ptr newUser(new User(username));
            server->getUsers()[username] = std::move(newUser); //Bota o user no map
        } else {
            user_ptr user(it->second);
            if(!user->canConnect()){
                // max connections exceeded
                conn->close();
                break;
            }
        }

        user_ptr user(server->getUsers()[username]);

        usersession_ptr session(new UserSession(conn, user));
        session->thread = std::thread(&UserSession::runSession, session.get());
        user->addSession(session);
    }

    std::cout << "Gracefully exiting?" << std::endl;
    return 0;
}
