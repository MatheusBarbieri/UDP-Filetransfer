#pragma once
#include <pwd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <map>

#include "util.hpp"

std::string findClientFolder(std::string username);
std::string setUpClientFolder(std::string username);
std::string findServerFolder();
std::string setUpServerFolder();
std::string findUserFolder(std::string username);
std::string setUpUserFolder(std::string username);

class Fileinfo {
public:
    std::string name;
    time_t mod;
    uint32_t size;
    Fileinfo();
};

typedef struct fileinfo {
    char name[256];
    uint32_t mod;
    uint32_t size;
} s_fileinfo;

std::map<std::string, Fileinfo> readFolder(std::string path);
void printFiles(std::map<std::string, Fileinfo> &files);
Fileinfo getFileinfo(std::string filepath);
