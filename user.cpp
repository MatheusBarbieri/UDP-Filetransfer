#include "user.hpp"

User::User(std::string username){
    this->username = username;
    this->userFolder = setUpUserFolder(username);
    this->files = readFolder(this->userFolder);
}

bool User::canConnect(){
    return (userSessions.size() < 2);
}

int User::startUserSession(User &user, UDPServer udpserver){
    return 0;
}

int User::endUserSession(){
    return 0;
}

uint32_t User::getFolderVersion(){
    return folderVersion;
}

std::string User::getUserFolder(){
    return userFolder;
}

std::string User::getUsername(){
    return username;
}
