#include <cstring>
#include "task.hpp"

Task::Task(int type){
    this->type = type;
}

Task::Task(int type, std::string info){
    this->type = type;
    this->info = info;
}

int Task::getType(){
    return type;
}

std::string Task::getInfo(){
    return info;
}
