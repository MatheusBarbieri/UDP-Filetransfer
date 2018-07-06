#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>
#include <list>

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
    std::list<udpconnection_ptr> serverAdresses;

public:
    Server();
    ~Server();
    bool existUser(std::string username);
    int createUser();
    std::map<std::string, user_ptr>& getUsers();

    void addConn(udpconnection_ptr conn);
    void master(int masterPort);
    void backup(int masterPort, std::string masterIp);
    std::string getUserNamesText();

    std::string getServerFolder();
};
