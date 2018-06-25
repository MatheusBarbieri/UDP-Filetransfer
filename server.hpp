#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "task.hpp"
#include "userSession.hpp"

class Server {
private:
    std::string serverFolder;
    std::vector<User> users;
    std::mutex usersMutex;
public:
    Server();

    bool existUser(std::string username);
    int addUser();

    std::string getServerFolder();
};
