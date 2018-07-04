#include "util.hpp"

Datagram createMessage(int type, std::string value){
    Datagram datagram;
    zerosDatagram(&datagram);
    datagram.type = type;
    int size = value.size();
    datagram.size = size;
    memcpy((void *) &datagram.data,
           (void *) value.c_str(),
           size
    );
    return datagram;
}

int generatePort(){
    static int port = 30000;
    return port++;
}

Semaphore::Semaphore() {
  if (initialized) {
    sem_destroy(&this->c_sem);
  }
  sem_init(&c_sem, 0, 0);
  initialized = true;
}

void Semaphore::init(int initValue) {
  if (initialized) {
    sem_destroy(&this->c_sem);
  }
  sem_init(&c_sem, 0, initValue);
  initialized = true;
}

Semaphore::~Semaphore(void) {
  int status = sem_destroy(&this->c_sem);
  if (status) {
      std::cout << "[Error] Could not destroy semaphore." << std::endl;
    } else {
    this->initialized = false;
  }
}

int Semaphore::wait() {
  int status = sem_wait(&this->c_sem);
  if (status) {
    std::cout << "[Error] Could not wait semaphore." << std::endl;
  }
  return status;
}

int Semaphore::post() {
  int status = sem_post(&this->c_sem);
  if (status) {
    std::cout << "[Error] Could not post semaphore." << std::endl;
  }
  return status;
}
