#pragma once
#include <pwd.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "util.hpp"

std::string getClientFolder(std::string username);
std::string setUpClientFolder(std::string username);

std::string getServerFolder();
std::string setUpServerFolder();

class Fileinfo {
public:
    std::string name;
    time_t mod;
    uint32_t size;
};
