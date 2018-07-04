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
    std::map<std::string, user_ptr> users;

public:
    Server();
    ~Server();
    bool existUser(std::string username);
    int createUser();
    std::map<std::string, user_ptr>& getUsers();

    std::string getServerFolder();
};
