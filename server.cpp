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

std::string Server::getServerFolder(){
    return serverFolder;
}

bool Server::existUser(std::string username){
    usersMutex.lock();
    for (auto &user : users){
        if (user.getUsername() == username){
            return true;
        }
    }
    usersMutex.unlock();
    return false;
}
