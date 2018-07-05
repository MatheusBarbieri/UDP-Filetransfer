#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "task.hpp"

#define IN_EVENT_SIZE (sizeof(struct inotify_event))
#define IN_BUF_LEN (64 * (IN_EVENT_SIZE + 16))

class Client {
private:
    std::string username;
    std::string clientFolder;

    std::mutex taskMutex;
    Semaphore taskAllocation;
    std::queue<Task> taskQueue;
    std::mutex filesMutex;
    std::map<std::string, Fileinfo> files;
    bool running;
public:
    UDPClient udpClient;

    Client(std::string username, UDPClient &udpclient);

    std::string getUsername();
    std::string getClientFolder();

    Task getTaskFromQueue();
    void addTaskToQueue(Task task);

    int connect();
    void startThreads();
    void inotifyLoop();
    void commandLoop();
    void taskManager();

    uint32_t getFolderVersion();

    void uploadFile(std::string filepath);
    void downloadFile(std::string filepath);
    void deleteFile(std::string filepath);
    bool exitTaskManager();
};
