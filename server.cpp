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

Server::~Server(){
  std::cout << "Deleting Server" << std::endl;
}

std::map<std::string, user_ptr>& Server::getUsers(){
    return users;
}

std::string Server::getServerFolder(){
    return serverFolder;
}
