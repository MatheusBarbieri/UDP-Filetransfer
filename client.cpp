#include "client.hpp"

Client::Client(std::string username, int port, std::string host); {
  this->userId = userId;
  this->userFolder = userFolder;
  this->ip = ip;
  this->port = port;
  this->socketDescriptor = this->loginServer();
}

void Client::startThreads(){
  thread inotifyLoop = thread(&Client::inotifyLoop, this);
  thread syncDirPoll = thread(&Client::syncDirPoll, this);
  thread commandLoop = thread(&Client::commandLoop, this);
  thread taskManager = thread(&Client::taskManager, this);
  taskManager.join();
}

void Client::syncDirPoll(){

}

void Client::commandLoop(){

}

void Client::taskManager(){

}

std::string Client::getUsername(){

}

std::string Client::getClientFolder(){

}

vector<Task> Client::getCommandFromQueue(){

}

void Client::addCommandToQueue(vector<Task> task){

}
