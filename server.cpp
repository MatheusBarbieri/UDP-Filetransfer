#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "server.hpp"

Server::Server(){
    this->serverFolder = setUpServerFolder();
}

std::map<std::string, User>& Server::getUsers(){
    return users;
}

std::string Server::getServerFolder(){
    return serverFolder;
}
