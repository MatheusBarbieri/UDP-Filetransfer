#pragma once
#include <cstring>
#include <semaphore.h>
#include <unistd.h>

#include "udp.hpp"

#define UPLOAD 7
#define DOWNLOAD 8
#define DELETE 9
#define CLIENT_LISTDIR 10
#define SERVER_LISTDIR 11
#define SYNCDIR 12
#define EXIT 13

Datagram createMessage(int type, std::string value);

class Semaphore {
public:
  Semaphore(int initValue);
  Semaphore(void);
  ~Semaphore(void);
  void init(int initValue);
  int post();
  int wait();
  private:;
  bool initialized;
  sem_t c_sem;
};

class Fileinfo {
public:
    std::string name;
    time_t mod;
    uint32_t size;
};
