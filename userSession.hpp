#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "user.hpp"

class User;

class UserSession {
private:
    User* user;
public:
    UDPServer udpServer;
    UserSession(UDPServer &udpserver, User* user);
    void runSession();
    uploadFile();
    downloadFile();
    deleteFile();
    listDir();
    dirVersion();
    exit();
};
