#pragma once
#include <cstring>
#include <semaphore.h>
#include <unistd.h>
#include <memory>

#include "udp.hpp"

#define UPLOAD 7
#define DOWNLOAD 8
#define DELETE 9
#define LOCALDIR 10
#define FOLDER_VERSION 11
#define SERVERDIR 12
#define EXIT 13
#define DECLINE 14
#define USERNAME 15

class Server;
class User;
class UserSession;

typedef std::shared_ptr<User> user_ptr;
typedef std::shared_ptr<Server> server_ptr;
typedef std::shared_ptr<UserSession> usersession_ptr;

int generatePort();
void vlog(const char*);

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
