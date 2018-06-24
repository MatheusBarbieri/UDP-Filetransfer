#include <thread>
#include <mutex>
#include <queue>
#include <algorithm>
#include <string>

#include "client.hpp"

Client::Client(std::string username, UDPClient &udpclient){
    this->udpClient = udpclient;
    this->username = username;
    this->clientFolder = setUpClientFolder(username);
}

void Client::startThreads(){
  std::thread inotifyLoop = std::thread(&Client::inotifyLoop, this);
  std::thread syncDirPoll = std::thread(&Client::syncDirPoll, this);
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
}

void Client::syncDirPoll(){
    Task syncdirTask(SYNCDIR);
    while(true) {
      usleep(10000000); //10 seconds
      addTaskToQueue(syncdirTask);
    }
}

void Client::commandLoop(){
    std::string command, filename;
    while (true) {
        std::cout << "> ";
        std::cin >> command;
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        if(command == "upload"){
            std::cin >> filename;
            Task task(DOWNLOAD, filename);
            addTaskToQueue(task);
        }
        else if(command == "download"){
            std::cin >> filename;
            Task task(UPLOAD, filename);
            addTaskToQueue(task);
        }
        else if(command == "delete"){
            std::cin >> filename;
            Task task(DELETE, filename);
            addTaskToQueue(task);
        }
        else if(command == "list_dir"){
            Task task(CLIENT_LISTDIR, filename);
            addTaskToQueue(task);
        }
        else if(command == "list_server"){
            Task task(SERVER_LISTDIR, filename);
            addTaskToQueue(task);
        }
        else if(command == "exit"){
            Task task(EXIT, filename);
            addTaskToQueue(task);
            std::cout << "Encerrando as operações..." << std::endl;
            break;
        }
    }
}

void Client::taskManager(){

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
