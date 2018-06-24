#include <thread>
#include <mutex>
#include <queue>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"

class Task;

class Client {
private:
    std::string username;
    std::string clientFolder;

    std::mutex taskMutex;
    Semaphore taskAllocation;
    std::queue<std::vector<Task> > taskQueue;
    bool queueEmpty;
public:
    UDPClient udpClient;

    Client (std::string username, int port, std::string host);

    std::string getUsername();
    std::string getClientFolder();

    std::vector<Task> getCommandFromQueue();
    void addCommandToQueue(std::vector<Task> task);

    void startThreads();
    void inotify();
    void syncDirPoll();
    void commandLoop();
    void taskManager();
};
