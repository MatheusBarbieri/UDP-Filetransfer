#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "userSession.hpp"

class UserSession;

class User{
private:
    std::string username;
    std::string userFolder;
    uint32_t folderVersion;
    std::map<std::string, Fileinfo> files;
    std::vector<UserSession> userSessions;
public:
    std::mutex actionMutex;

    User(std::string username);

    bool canConnect();
    int startUserSession(User &user, UDPServer udpserver);
    int endUserSession();

    uint32_t getFolderVersion();
    std::string getUserFolder();
    std::string getUsername();
};
