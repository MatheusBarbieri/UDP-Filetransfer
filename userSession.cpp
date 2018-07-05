#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>

#include "userSession.hpp"

UserSession::UserSession(udpconnection_ptr udpconnection, user_ptr user){
    this->user = std::move(user);
    this->udpConnection = std::move(udpconnection);
}

UserSession::~UserSession(){
  std::cout << "Deleting UserSession" << std::endl;
}

void UserSession::runSession(){
    int status = 0;
    bool running = true;
    Datagram* message = udpConnection->getRecvbuffer();
    zerosDatagram(message);
    while(running){
        status = udpConnection->recDatagram();
        if (status >= 0){
            user->actionMutex.lock();
            switch (message->type) {
                case UPLOAD:
                {
                    std::cerr << "UPLOAD (receiving file)" << '\n';
                    Datagram dg;
                    s_fileinfo *sinfo = (s_fileinfo*) message->data;
                    Fileinfo info;
                    info.mod = ntohl(sinfo->mod);
                    info.size = ntohl(sinfo->size);
                    info.name = sinfo->name;
                    auto it = user->files.find(info.name);
                    if (it != user->files.end()){
                        if (info.mod == it->second.mod && info.size == it->second.size){
                            dg.type = DECLINE;
                            dg.seqNumber = 0;
                            dg.size = 0;
                            udpConnection->sendDatagram(dg);
                            break;
                        }
                    }
                    dg.type = ACCEPT;
                    dg.seqNumber = 0;
                    dg.size = 0;
                    udpConnection->sendDatagram(dg);

                    std::string filepath = user->userFolder + info.name;
                    FILE* file = fopen(filepath.c_str(), "wb");
                    udpConnection->receiveFile(file);
                    fclose(file);

                    // send file's new timestap
                    info = getFileinfo(filepath);
                    sinfo->mod = htonl(info.mod);
                    sinfo->size = htonl(info.size);
                    udpConnection->sendDatagram(dg);
                    break;
                }
/*
                    int status;
                    Fileinfo info = getFileinfo(filepath);
                    Datagram dg;
                    Datagram* dgRcv;
                    s_fileinfo *sinfo = (s_fileinfo*) dg.data;
                    sinfo->mod = htonl(info.mod);
                    sinfo->size = htonl(info.size);
                    strncpy(sinfo->name, info.name.c_str(), sizeof(sinfo->name));
                    dg.type = UPLOAD;
                    dg.seqNumber = 0;
                    dg.size = sizeof(s_fileinfo);
                    udpClient.sendDatagram(dg);
                    status = udpClient.recDatagram();
                    dgRcv = udpClient.getRecvbuffer();
                    if (dgRcv->type == DECLINE) {
                        return;
                    }
                    FILE* file = fopen(filepath.c_str(), "rb");
                    udpClient.sendFile(file);
                    status = udpClient.recDatagram();
                    dgRcv = udpClient.getRecvbuffer();
                    struct utimbuf modTime;
                    modTime.modtime = ntohl(sinfo->mod);
                    modTime.actime = modTime.modtime;
                    utime(info.name.c_str(), &modTime);
                    return;*/
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
                    udpConnection->sendMessage((char*) info, sizeof(s_fileinfo) * numFiles);
                    free(info);
                } break;
                case FOLDER_VERSION:
                    message->seqNumber = user->getFolderVersion();
                    udpConnection->sendDatagram(*message);
                    break;
                case EXIT:
                    udpConnection->sendDatagram(*message);
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
