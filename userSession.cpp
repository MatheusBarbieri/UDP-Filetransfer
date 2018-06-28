#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "userSession.hpp"

UserSession::UserSession(UDPServer udpserver, User* user){
    this->user = user;
    this->udpServer = udpserver;
}

UserSession::~UserSession(){
  std::cout << "Deleting UserSession" << std::endl;
}

void UserSession::runSession(){
    int status;
    bool running = true;
    Datagram* message = udpServer.getRecvbuffer();
    zerosDatagram(message);
    while(running){
        status = udpServer.recDatagram();
        if (status == 0){
            user->actionMutex.lock();
            switch (message->type) {
                case UPLOAD:

                    break;
                case DOWNLOAD:

                    break;
                case DELETE:

                    break;
                case SERVERDIR: {
                    int numFiles = user->files.size();

                    s_fileinfo *info = (s_fileinfo*)calloc(numFiles, sizeof(s_fileinfo));
                    int i = 0;
                    for (auto const ent : user->files) {
                        Fileinfo fileinfo = ent.second;
                        strncpy(info[i].name, fileinfo.name.c_str(), 255);
                        info[i].mod = htonl(fileinfo.mod);
                        info[i].size = htonl(fileinfo.size);
                        i += 1;
                    }
                    udpServer.sendMessage((char*) info, sizeof(s_fileinfo) * numFiles);
                    free(info);
                } break;
                case FOLDER_VERSION:
                    message->seqNumber = user->getFolderVersion();
                    udpServer.sendDatagram(*message);
                    break;
                case EXIT:
                    udpServer.sendDatagram(*message);
                    running = false;
                    break;
                default:
                    std::cout << "Invalid Request." << std::endl;
                    break;
            }
            user->actionMutex.unlock();
        } else if (status == TIMEOUT){
            std::cout << "Response timed out." << std::endl;
        } else {
            std::cout << "Error on receiving datagram, exiting process." << std::endl;
            running = false;
        }
    }
}
