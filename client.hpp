#pragma once
#include <thread>
#include <mutex>
#include <queue>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "task.hpp"

class UDPClient;

class Client {
private:
    std::string username;
    std::string clientFolder;

    std::mutex taskMutex;
    Semaphore taskAllocation;
    std::queue<Task> taskQueue;
public:
    UDPClient udpClient;

    Client(std::string username, UDPClient &udpclient);

    std::string getUsername();
    std::string getClientFolder();

    Task getTaskFromQueue();
    void addTaskToQueue(Task task);

    void startThreads();
    void inotifyLoop();
    void syncDirPoll();
    void commandLoop();
    void taskManager();
};
