#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sstream>
#include <sys/inotify.h>

#include "server.hpp"

Server::Server(){
    this->serverFolder = setUpServerFolder();
}

Server::~Server(){
    std::cout << "Deleting Server" << std::endl;
}

std::map<std::string, user_ptr>& Server::getUsers(){
    return users;
}

std::string Server::getServerFolder(){
    return serverFolder;
}

void Server::addConn(udpconnection_ptr conn){
    serverAdresses.push_back(conn);
}

std::string Server::getUserNamesText(){
    std::stringstream names;
    for (const auto& user : users){
        names << user.first << std::endl;
    }
    return names.str();
}

void Server::master(int masterPort){
    UDPServer server(masterPort);
    server._bind();
    Datagram dg;
    while(true){
        udpconnection_ptr conn = server.accept();
        addConn(conn);
        std::string names = getUserNamesText();
        conn->sendString(names);

        for (const auto& user: users){
            memcpy(&dg.data, user.first.c_str(), user.first.size());
            user_ptr currentUser = user.second;
            dg.seqNumber = currentUser->files.size();
            conn->sendDatagram(dg);

            for(const auto& file : currentUser->files){
                zerosDatagram(&dg);
                s_fileinfo* sinfo = (s_fileinfo*) dg.data;
                memcpy(sinfo->name, file.first.c_str(), file.first.size());
                sinfo->mod = htonl(file.second.mod);
                sinfo->size = htonl(file.second.size);
                conn->sendDatagram(dg);

                std::string filePath = currentUser->getUserFolder() + currentUser->getUsername();
                FILE* fp = fopen(filePath.c_str(), "rb");
                conn->sendFile(fp);
                fclose(fp);
            }
        }
    }
}

void Server::backup(int masterPort, std::string masterIp){
    UDPClient backup(masterPort, masterIp);

    int response = backup.connect();

    std::cout << "Response: " << response << std::endl;
    if (response == ACCEPT){
        std::cout << "conectou com master com sucesso!" << std::endl;
    } else {
        std::cout << "Não foi possível conectar com master." << std::endl;
        return;
    }

    // get users.
    std::string usernames = backup.receiveString();
    std::stringstream names(usernames);
    for (std::string name; std::getline(names, name); ) {
        users[name] = user_ptr(new User(name));
    }
    int numUsers = users.size();

    // get files
    for (int i = 0; i < numUsers; i++) {
        backup.recDatagram();
        Datagram *dg = backup.getRecvbuffer();
        int numFiles = dg->seqNumber;
        std::string username = dg->data;

        user_ptr currentUser = users[username];

        for (int j = 0; j < numFiles; j++) {
            backup.recDatagram();
            s_fileinfo *sinfo = (s_fileinfo*) dg->data;
            Fileinfo info;
            info.size = ntohl(sinfo->size);
            info.mod = ntohl(sinfo->mod);
            info.name = sinfo->name;
            std::string filePath = currentUser->getUserFolder() + username;
            FILE* fp = fopen(filePath.c_str(), "wb");
            backup.receiveFile(fp);
            fclose(fp);
            currentUser->files[info.name] = info;
        }
    }

    // TODO: wait updates
}
