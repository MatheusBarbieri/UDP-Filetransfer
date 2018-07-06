#include <mutex>
#include <queue>
#include <algorithm>
#include <string>
#include <regex>
#include <thread>
#include <sys/inotify.h>
#include <sys/types.h>
#include <utime.h>
#include <ctime>

#include "client.hpp"

Client::Client(std::string username, UDPClient &udpclient){
    this->udpClient = udpclient;
    this->username = username;
    this->clientFolder = setUpClientFolder(username);
    this->folderVersion = 0;
    files = readFolder(clientFolder);
}

void Client::startThreads(){
  running = true;
  std::thread inotifyLoop = std::thread(&Client::inotifyLoop, this);
  std::thread commandLoop = std::thread(&Client::commandLoop, this);
  std::thread syncDirLoop = std::thread(&Client::syncDirLoop, this);
  std::thread taskManager = std::thread(&Client::taskManager, this);
  taskManager.join();
}

int Client::connect(){
    vlog("Client tries connect");
    udpClient.connect();
    vlog("Client waiting response");
    int response = udpClient.waitResponse();
    vlog("Client got response");
    if (response == ACCEPT){
        vlog("É um accept");
        Datagram userName;
        zerosDatagram(&userName);
        userName.type = USERNAME;
        char* name = (char*) username.c_str();
        memcpy(userName.data, name, username.size());
        vlog("Enviando o nome: dg3");
        udpClient.sendDatagram(userName);
        vlog("Esperando validação do nome");

        udpClient.recDatagram();
        Datagram* response = udpClient.getRecvbuffer();
        return response->type;
    }
    return REJECT;
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
            addTaskToQueue(Task(UPLOAD, filename));
        }
        else if(command == "download"){
            std::cin >> filename;
            addTaskToQueue(Task(DOWNLOAD, filename));
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
        else {
            std::cout << "Invalid operation." << std::endl;
        }
    }
}


void Client::syncDirLoop(){
    while(running){
        addTaskToQueue(Task(SYNCDIR));
        sleep(10);
        vlog("Sync Dir Loop!");
    }
}


uint32_t Client::getFolderVersion(){
    Datagram message, *recData;
    message.type = FOLDER_VERSION;
    udpClient.sendDatagram(message);
    int status = udpClient.recDatagram();
    recData = udpClient.getRecvbuffer();
    if (status != 0 and recData->type == FOLDER_VERSION){
        return recData->seqNumber;
    }
    return -1;
}

bool Client::exitTaskManager(){
    Datagram *recData;
    int status = udpClient.recDatagram();
    recData = udpClient.getRecvbuffer();
    if (status == 0 and recData->type == EXIT){
        return false;
    }
    return true;
}

void Client::listLocalDirectory(){
    std::cout << "Pasta do usuário local:\n";
    std::map<std::string, Fileinfo> fileList = readFolder(getClientFolder());
    printFiles(fileList);
}

void Client::listRemoteDirectory(){
    std::cout << "Pasta do usuário remota:\n";
    std::map<std::string, Fileinfo> fileList = getRemoteDirectory();
    printFiles(fileList);
}

std::map<std::string, Fileinfo> Client::getRemoteDirectory(){
    Datagram message, *recData;
    char* fileInfos;
    int numFiles;
    message.type = SERVERDIR;
    udpClient.sendDatagram(message);
    udpClient.recDatagram();
    recData = udpClient.getRecvbuffer();
    numFiles = recData->seqNumber;
    fileInfos = udpClient.receiveMessage();
    int totalSize = udpClient.getRecvMessageSize();

    std::map<std::string, Fileinfo> fileList;

    std::cout << "Number of files: " << numFiles << std::endl;
    std::cout << "TotalSize: " << totalSize << std::endl;
    std::cout << "struct size: " << sizeof(s_fileinfo) << std::endl;

    s_fileinfo* readInfo = (s_fileinfo*) fileInfos;
    Fileinfo info;
    std::cout << "Teste 1" << std::endl;
    for(int i=0; i<numFiles; i++){
        std::cout << "Teste 2: " << i << std::endl;
        std::string fileName(readInfo->name);
        std::cout << "Teste 3: " << fileName << std::endl;
        info.name = fileName;
        std::cout << "Teste 4: " << info.name << std::endl;
        info.mod = readInfo->mod;
        std::cout << "Teste 5: " << info.mod << std::endl;
        info.size = readInfo->size;
        std::cout << "Teste 6: " << info.size << std::endl;
        fileList[fileName] = info;
        std::cout << "Teste 7" << std::endl;
        readInfo++;
    }
    return fileList;
}


void Client::uploadFile(std::string filepath){
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
    udpClient.recDatagram();
    dgRcv = udpClient.getRecvbuffer();
    if (dgRcv->type != ACCEPT) {
        return;
    }
    FILE* file = fopen(filepath.c_str(), "rb");
    udpClient.sendFile(file);
    fclose(file);
    udpClient.recDatagram();
    dgRcv = udpClient.getRecvbuffer();
    struct utimbuf modTime;
    modTime.modtime = ntohl(sinfo->mod);
    modTime.actime = modTime.modtime;
    utime(filepath.c_str(), &modTime);
    info.mod = modTime.modtime;
    files[info.name] = info;
    return;
}

void Client::downloadFile(std::string filepath){
    Datagram dg;
    Datagram *dgRcv;
    Fileinfo info;
    s_fileinfo *sinfo = (s_fileinfo*) dg.data;
    std::string filename = filenameFromPath(filepath);
    std::string foldername = dirnameFromPath(filepath);
    foldername = foldername + '/';
    info.name = filename;
    sinfo->size = 0;
    sinfo->mod = 0;
    strncpy(sinfo->name, filename.c_str(), 255);
    dg.type = DOWNLOAD;
    dg.seqNumber = 0;
    dg.size = sizeof(s_fileinfo);
    udpClient.sendDatagram(dg);
    udpClient.recDatagram();
    dgRcv = udpClient.getRecvbuffer();
    if (dgRcv->type != ACCEPT) {
        return;
    }
    sinfo = (s_fileinfo*) dgRcv->data;

    info.mod = ntohl(sinfo->mod);
    info.size = ntohl(sinfo->size);

    FILE* file = fopen(filepath.c_str(), "wb");
    udpClient.receiveFile(file);
    fclose(file);

    struct utimbuf modTime;
    modTime.modtime = info.mod;
    modTime.actime = info.mod;
    utime(filepath.c_str(), &modTime);
    if (foldername.compare(clientFolder) == 0) {
        files[info.name] = info;
    }
    return;
}

void Client::syncDir(){
    int remoteFolderVersion = getFolderVersion();
    std::map<std::string, Fileinfo> remoteHistory;

    if (folderVersion < remoteFolderVersion){
        std::cout << "VersionsL: " << folderVersion << std::endl;
        std::cout << "VersionsR: " << remoteFolderVersion << std::endl;
        remoteHistory = getRemoteDirectory();

        for(const auto& remoteFile : remoteHistory){
            std::string remoteFileName = remoteFile.first;

            auto it = files.find(remoteFileName);
            if(it == files.end()){ //IF file does not exists
                addTaskToQueue(Task(DOWNLOAD, remoteFileName));
            } else { //IF file exists
                if(remoteFile.second.mod > it->second.mod){
                    addTaskToQueue(Task(DOWNLOAD, remoteFileName));
                }
            }
        }

        remoteHistory = getRemoteDirectory();
        for(const auto& localFile : files){
            std::string localFileName = localFile.first;
            std::cout << "Teste: " << localFileName << std::endl;
            auto it = remoteHistory.find(localFileName);
            if(it == remoteHistory.end()){ //IF file does not exists
                std::cout << "Tentou apagar" << std::endl;
                auto eraseIt = files.find(localFileName);
                files.erase(eraseIt);
                std::string filepath = getClientFolder() + localFileName;
                remove(filepath.c_str());
            }
        }
    }

    folderVersion = remoteFolderVersion;
}

void Client::deleteFile(std::string filename){
    filename = filenameFromPath(filename);
    std::string filepath = clientFolder + filename;
    Datagram dg;
    s_fileinfo *sinfo = (s_fileinfo*) dg.data;
    strncpy(sinfo->name, filename.c_str(), 255);
    sinfo->size = 0;
    sinfo->mod = 0;
    dg.type = DELETE;
    dg.seqNumber = 0;
    dg.size = 0;
    udpClient.sendDatagram(dg);
    Datagram *dgRcv;
    udpClient.recDatagram();
    dgRcv = udpClient.getRecvbuffer();

    if (dgRcv->type == ACCEPT && access(filepath.c_str(), F_OK) != -1) {
        remove(filepath.c_str());
    }
}

void Client::taskManager(){
    while(running){
        Task task = getTaskFromQueue();
        switch (task.getType()) {
            case DOWNLOAD:
                std::cout << "Fazendo download do arquivo: " << task.getInfo() << std::endl;
                downloadFile(task.getInfo());
                break;
            case UPLOAD:
                std::cout << "Fazendo upload do arquivo: " << task.getInfo() << std::endl;
                uploadFile(task.getInfo());
                break;
            case DELETE:
                deleteFile(task.getInfo());
                std::cout << "Deletou: " << task.getInfo() << std::endl;
                break;
            case LOCALDIR:
                listLocalDirectory();
                break;
            case SERVERDIR:
                listRemoteDirectory();
                break;
            case SYNCDIR:
                syncDir();
                break;
            case EXIT:
                running = exitTaskManager();
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
