#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "task.hpp"

class Server {
private:
    std::string serverFolder;
public:
    UDPServer udpServer;

    Server(UDPServer &udpserver);

    std::string getServerFolder();
};
