#pragma once
#include <thread>
#include <mutex>
#include <queue>
#include <map>

#include "udp.hpp"
#include "util.hpp"
#include "filesystem.hpp"
#include "userSession.hpp"

class UserSession;

class User{
friend class UserSession;
protected:
    std::string username;
    std::string userFolder;
    uint32_t folderVersion;
    std::vector<usersession_ptr> userSessions;
public:
    std::map<std::string, Fileinfo> files;
    std::mutex actionMutex;

    User(std::string username);
    ~User();

    bool canConnect();
    int addSession(usersession_ptr userSession);
    int endUserSession();

    uint32_t getFolderVersion();
    void bumpFolderVersion();
    std::string getUserFolder();
    std::string getUsername();
};
