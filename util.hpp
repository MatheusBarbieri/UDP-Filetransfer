#pragma once
#include <cstring>
#include <semaphore.h>
#include <unistd.h>

#include "udp.hpp"

#define UPLOAD 7
#define DOWNLOAD 8
#define DELETE 9
#define LOCALDIR 10
#define FOLDER_VERSION 11
#define SERVERDIR 12
#define EXIT 13
#define TIMEOUT 404

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
