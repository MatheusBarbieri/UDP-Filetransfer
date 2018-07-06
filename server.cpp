#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sstream>
#include <sys/inotify.h>

#include "server.hpp"

Server::Server(){
    this->serverFolder = setUpServerFolder();
}

Server::~Server(){
  std::cout << "Deleting Server" << std::endl;
}

std::map<std::string, user_ptr>& Server::getUsers(){
    return users;
}

std::string Server::getServerFolder(){
    return serverFolder;
}

void Server::addConn(udpconnection_ptr conn){
    serverAdresses.push_back(conn);
}

std::string Server::getUserNamesText(){
    std::stringstream names;
    for (const auto& user : users){
        names << user.first << std::endl;
    }
    return names.str();
}

void Server::master(int masterPort){
    UDPServer server(masterPort);
    server._bind();
    while(true){
        udpconnection_ptr conn = server.accept();
        addConn(conn);
        std::string names = getUserNamesText();
        conn->sendString(names);
    }
}
