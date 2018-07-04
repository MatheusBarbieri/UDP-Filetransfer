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
    user_ptr user;
public:
    ~UserSession();
    udpserver_ptr udpServer;
    UserSession(udpserver_ptr rudpserver, user_ptr user);
    void runSession();

    uint32_t uploadFile(std::string filename);
    Fileinfo downloadFile(std::string filename);
    uint32_t deleteFile(std::string filename);
    uint32_t listDir();
    uint32_t dirVersion();
    uint32_t exit();
};
