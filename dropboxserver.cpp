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
    std::string masterIp;

    if (argc < 3) {
        std::cout << "Usage:\n\t./udpServer <port> <type: master/slave> <master_port> <ip(if slave)>" << std::endl;
        return 0;
    }
    bool isBackup = false;
    int port = atoi(argv[1]);
    std::string serverType = argv[2];
    int masterPort = atoi(argv[3]);
    if (argc == 5){
        masterIp = argv[4];
        isBackup = true;
    }

    server_ptr server(new Server);

    if (!isBackup) {
        std::thread masterServer = std::thread(&Server::master, server.get(), masterPort);
    } else {
        std::thread backupServer = std::thread(&Server::backup, server.get(), masterPort, masterIp);
    }

    udpserver_ptr udpserver(new UDPServer(port));
    udpserver->_bind();

    while(true){
        vlog("Server waiting accept");
        udpconnection_ptr conn = udpserver->accept(); //wait for new users
        vlog("Accept sucess. waiting for username");
        conn->recDatagram();
        vlog("Got username");
        Datagram* userName = conn->getRecvbuffer();
        std::string username(userName->data);

        user_ptr user;
        Datagram response;

        auto it = server->getUsers().find(username);
        if (it == server->getUsers().end()) {
            //IF USER DO NOT EXIST
            user_ptr newUser(new User(username));
            server->getUsers()[username] = newUser;

            response.type = ACCEPT;
            conn->sendDatagram(response);
            usersession_ptr session(new UserSession(conn, newUser));
            newUser->addSession(session);
            session->thread = std::thread(&UserSession::runSession, session.get());
        } else {
            //IF USER EXISTS
            user_ptr user(it->second);
            if(!user->canConnect()){
                //IF USER ALREADY HAS 2 ACTIVE SESSIONS
                response.type = REJECT;
                conn->sendDatagram(response);
                conn->close();
                continue;
            } else {
                //IF USER CAN CONNECT
                response.type = ACCEPT;
                conn->sendDatagram(response);
                usersession_ptr session(new UserSession(conn, user));
                user->addSession(session);
                session->thread = std::thread(&UserSession::runSession, session.get());
            }
        }
    }

    std::cout << "Gracefully exiting?" << std::endl;
    return 0;
}
