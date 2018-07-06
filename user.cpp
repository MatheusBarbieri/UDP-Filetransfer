#include "user.hpp"

User::User(std::string username){
    this->username = username;
    this->userFolder = setUpUserFolder(username);
    this->folderVersion = 1;
    this->files = readFolder(this->userFolder);
}

User::~User(){
  std::cout << "Deleting User" << std::endl;
}

bool User::canConnect(){
    return (userSessions.size() < 2);
}

int User::addSession(usersession_ptr userSession){
    userSessions.push_back(userSession);
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
