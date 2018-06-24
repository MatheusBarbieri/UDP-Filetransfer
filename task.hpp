#pragma once
#include <stdlib.h>
#include <cstring>
#include <memory>

#include "udp.hpp"

class Task {
private:
    int type;
    std::string info;
public:
    Task(int type);
    Task(int type, std::string info);
    int getType();
    std::string getInfo();
};
