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
    std::mutex usersMutex;
    std::map<std::string, User> users;

public:
    Server();
    ~Server();
    bool existUser(std::string username);
    int createUser();
    std::map<std::string, User>& getUsers();

    std::string getServerFolder();
};
