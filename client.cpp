#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "client.hpp"

Client::Client(std::string username, UDPClient &udpclient){
    this->udpClient = udpclient;
    this->username = username;
    this->clientFolder = setUpClientFolder(username);
    files = readFolder(clientFolder);
}

void Client::startThreads(){
  running = true;
  std::thread inotifyLoop = std::thread(&Client::inotifyLoop, this);
  std::thread commandLoop = std::thread(&Client::commandLoop, this);
  std::thread taskManager = std::thread(&Client::taskManager, this);
  taskManager.join();
}

std::string Client::getUsername(){
    return username;
}

std::string Client::getClientFolder(){
    return clientFolder;
}

void Client::inotifyLoop(){
    std::regex ignore_regex("(4913|.*\\.fstmp|.*\\.swpx{0,1}|.*~)$");
    // start inotify
    int fd = inotify_init();
    if (fd < 0) {
        return;
    }
    int wd = inotify_add_watch(fd, clientFolder.c_str(),
    IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MOVED_TO | IN_MOVED_FROM |
    IN_DONT_FOLLOW | IN_EXCL_UNLINK);
    if (wd < 0) {
        return;
    }
    ssize_t len = 0;
    char buffer[IN_BUF_LEN];

    while (running) {
        // read events
        len = read(fd, buffer, IN_BUF_LEN);
        if (len < 0) {
            switch (errno) {
                case EINTR:
                // interrupted by signal
                // exit
                close(fd);
                return;
                break;
                default:
                std::cerr << "inotify problem" << std::endl;
                break;
            }
        }
        ssize_t i = 0;
        // read inotify events
        while (i < len) {
            struct inotify_event *event;
            event = (struct inotify_event *) &buffer[i];
            std::string filename = event->name;
            std::string filepath = clientFolder + "/" + filename;
            if(std::regex_match(filename, ignore_regex)) {
                // ignored
            } else {
                switch (event->mask) {
                    case IN_CREATE: // new file
                    case IN_MOVED_TO: // moved into folder
                    case IN_CLOSE_WRITE: { // modified
                        addTaskToQueue(Task(UPLOAD, filepath));
                    } break;
                    case IN_DELETE: // deleted
                    case IN_MOVED_FROM: { // moved from folder
                        filesMutex.lock();
                        auto it = files.find(filename);
                        if (it != files.end()) {
                            addTaskToQueue(Task(DELETE, filename));
                        }
                        filesMutex.unlock();
                    } break;
                }
            }
            i += IN_EVENT_SIZE + event->len;
        }
    }
    close(fd);
}

void Client::commandLoop(){
    std::string command, filename;
    while (true) {
        std::cout << "> ";
        std::cin >> command;
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        if(command == "upload"){
            std::cin >> filename;
            addTaskToQueue(Task(DOWNLOAD, filename));
        }
        else if(command == "download"){
            std::cin >> filename;
            addTaskToQueue(Task(UPLOAD, filename));
        }
        else if(command == "delete"){
            std::cin >> filename;
            addTaskToQueue(Task(DELETE, filename));
        }
        else if(command == "list_dir"){
            addTaskToQueue(Task(LOCALDIR, filename));
        }
        else if(command == "list_server"){
            addTaskToQueue(Task(SERVERDIR, filename));
        }
        else if(command == "exit"){
            addTaskToQueue(Task(EXIT, filename));
            std::cout << "Encerrando as operações..." << std::endl;
            break;
        }
    }
}

void Client::taskManager() {
    bool running = true;
    while(running){
        Datagram message, *recData;
        recData = (Datagram*) &udpClient.recvbuffer;
        int status;
        Task task = getTaskFromQueue();
        switch (task.getType()) {
            case DOWNLOAD:

                break;
            case UPLOAD:

                break;

            case DELETE:

                break;
            case LOCALDIR:

                break;
            case SERVERDIR:

                break;
            case EXIT:
                message.type = EXIT;
                status = udpClient.sendDatagram(message);
                if (status == 0){
                    status = udpClient.recDatagram();
                }
                if (status == 0 and recData->type == EXIT){
                    running = false;
                }
                break;
        }
    }
}

Task Client::getTaskFromQueue() {
  this->taskAllocation.wait();
  this->taskMutex.lock();
  Task task = this->taskQueue.front();
  this->taskQueue.pop();
  this->taskMutex.unlock();
  return task;
}

void Client::addTaskToQueue(Task task) {
  this->taskAllocation.post();
  this->taskMutex.lock();
  this->taskQueue.push(task);
  this->taskMutex.unlock();
}
