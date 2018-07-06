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
                            std::cerr << "UPLOAD (END)" << '\n';
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
                    sinfo = (s_fileinfo*) dg.data;
                    sinfo->mod = htonl(info.mod);
                    sinfo->size = htonl(info.size);
                    udpConnection->sendDatagram(dg);

                    // add file to filelist
                    user->files[info.name] = info;
                    user->bumpFolderVersion();
                    break;
                    std::cerr << "UPLOAD (END)" << '\n';
                }
                case DOWNLOAD:
                {
                    std::cerr << "DOWNLOAD (sending file)" << '\n';
                    Datagram dg;
                    s_fileinfo *sinfo = (s_fileinfo*) message->data;
                    Fileinfo info;
                    info.name = sinfo->name;

                    auto it = user->files.find(info.name);
                    if (it == user->files.end()) {
                        dg.type = DECLINE;
                        dg.seqNumber = 0;
                        dg.size = 0;
                        udpConnection->sendDatagram(dg);
                        std::cerr << "DOWNLOAD (END)" << '\n';
                        break;
                    }
                    info = it->second;

                    std::string filepath = user->userFolder + info.name;
                    FILE* file = fopen(filepath.c_str(), "rb");
                    if (!file) {
                        dg.type = DECLINE;
                        dg.seqNumber = 0;
                        dg.size = 0;
                        udpConnection->sendDatagram(dg);
                        std::cerr << "DOWNLOAD (END)" << '\n';
                        break;
                    }

                    dg.type = ACCEPT;
                    sinfo = (s_fileinfo*) dg.data;
                    sinfo->size = htonl(info.size);
                    sinfo->mod = htonl(info.mod);
                    udpConnection->sendDatagram(dg);
                    udpConnection->sendFile(file);
                    fclose(file);

                    std::cerr << "DOWNLOAD (END)" << '\n';
                } break;
                case DELETE: {
                    std::cerr << "DELETE (purging file)" << '\n';
                    s_fileinfo *sinfo = (s_fileinfo*) message->data;
                    Fileinfo info;
                    info.name = sinfo->name;
                    std::string filepath = user->userFolder + info.name;

                    auto it = user->files.find(info.name);
                    if (it != user->files.end()) {
                        user->files.erase(it);
                    }

                    remove(filepath.c_str());

                    Datagram dg;
                    dg.type = ACCEPT;
                    dg.seqNumber = 0;
                    dg.size = 0;
                    udpConnection->sendDatagram(dg);
                    user->bumpFolderVersion();

                    std::cerr << "DELETE (end)" << '\n';
                } break;
                case SERVERDIR: {
                    std::cerr << "SERVERDIR (sending list of files)" << '\n';
                    int numFiles = user->files.size();
                    if (numFiles <= 0) {
                        Datagram numFilesDatagram;
                        numFilesDatagram.type = SERVERDIR;
                        numFilesDatagram.seqNumber = 0;
                        udpConnection->sendDatagram(numFilesDatagram);
                        std::cerr << "SERVERDIR (end)" << '\n';
                        break;
                    }

                    s_fileinfo *info = (s_fileinfo*)calloc(numFiles, sizeof(s_fileinfo));
                    memset(info, 0, numFiles * sizeof(s_fileinfo));
                    int i = 0;
                    for (auto const fileIt : user->files) {
                        Fileinfo fileinfo = fileIt.second;
                        strncpy(info[i].name, fileinfo.name.c_str(), 255);

                        info[i].mod = htonl(fileinfo.mod);
                        info[i].size = htonl(fileinfo.size);
                        i += 1;
                    }
                    Datagram numFilesDatagram;
                    numFilesDatagram.type = SERVERDIR;
                    numFilesDatagram.seqNumber = i;
                    udpConnection->sendDatagram(numFilesDatagram);
                    udpConnection->sendMessage((char*) info, sizeof(s_fileinfo) * numFiles);
                    free(info);
                    std::cerr << "SERVERDIR (end)" << '\n';
                } break;
                case FOLDER_VERSION:
                    std::cerr << "FOLDER_VERSION (start)" << '\n';
                    message->seqNumber = user->getFolderVersion();
                    udpConnection->sendDatagram(*message);
                    std::cerr << "FOLDER_VERSION (end)" << '\n';
                    break;
                case EXIT:
                    std::cout << "Exiting Server!" << std::endl;
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
