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

std::string getServerFolder(){
    const char *homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    std::string dir = std::string(homedir) + "/dropboxServer";
    return dir;
}

std::string setUpServerFolder(){
    std::string dir = getServerFolder();
    const char* folder = dir.c_str();
    struct stat sb;
    if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)){
        std::cout << "Server Folder found." << std::endl;
    } else {
        mkdir(folder, S_IRWXU | S_IRWXG | S_IRWXO);
        std::cout << "No folder found for client.\nCreating Folder:\n\t" << dir << std::endl;
    }
    return dir;
}

std::map<std::string, Fileinfo> readFolder(std::string path) {
    std::map<std::string, Fileinfo> files;
    DIR* dirp = opendir(path.c_str());
    struct dirent *entry;
    entry = readdir(dirp);
    while (entry) {
      std::string filename(entry->d_name);
      std::string filepath = path + "/" + filename;
      struct stat path_stat;
      stat(filepath.c_str(), &path_stat);
      if (S_ISREG(path_stat.st_mode)) {
        Fileinfo info;
        info.mod = path_stat.st_mtime;
        info.size = path_stat.st_size;
        info.name = entry->d_name;
        files[filename] = info;
      }
      entry = readdir(dirp);
    }
    return files;
}

void printFiles(std::map<std::string, Fileinfo> &files) {
    for (auto it = files.begin(); it != files.end(); it++) {
        Fileinfo info = (*it).second;
        std::cerr << " - " << info.name << '\n';
    }
}
