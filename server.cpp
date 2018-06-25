#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "server.hpp"

Server::Server(UDPServer &udpserver){
    this->udpServer = udpserver;
    this->serverFolder = setUpServerFolder();
}

std::string Server::getServerFolder(){
    return serverFolder;
}
