#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "userSession.hpp"

UserSession::UserSession(UDPServer &udpserver, User* user){
    this->user = user;
    this->udpServer = udpserver;
}

void UserSession::runSession(){
    int status;
    bool running = true;
    Datagram message;
    zerosDatagram(&message);
    while(running){
        status = udpServer.recDatagram();
        if (status == 0){
            memcpy(&message, (const void*) &udpServer.recvbuffer, DATASIZE);
            user->actionMutex.lock();
            switch (message.type) {
                case UPLOAD:

                    break;
                case DOWNLOAD:

                    break;
                case DELETE:

                    break;
                case SERVERDIR:

                    break;
                case FOLDER_VERSION:

                    break;
                case EXIT:
                    running = false;
                    break;
                default:
                    std::cout << "Invalid Request." << std::endl;
                    break;
            }
            user->actionMutex.unlock();
        } else {
            std::cout << "Error on receiving datagram, exiting process." << std::endl;
            running = 0;
        }
    }
}
