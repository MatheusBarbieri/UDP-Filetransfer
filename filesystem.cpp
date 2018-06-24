#include "filesystem.hpp"

std::string getClientFolder(std::string username){
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    std::string dir = std::string(homedir) + "/sync_dir_" + username;
    return dir;
}

std::string setUpClientFolder(std::string username){
    std::string dir = getClientFolder(username);
    const char* folder = dir.c_str();
    struct stat sb;
    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)){
        std::cout << "Client Folder found." << std::endl;
    } else {
        mkdir(folder, S_IRWXU | S_IRWXG | S_IRWXO);
        std::cout << "No folder found for client.\nCreating Folder:\n\t" << dir << std::endl;
    }
    return dir;
}
