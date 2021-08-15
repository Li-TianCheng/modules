//
// Created by ltc on 2021/3/7.
//

#include "Mutex.h"
#include <stdexcept>

Mutex::Mutex():mutex() {
    if (pthread_mutex_init(&mutex, nullptr) != 0){
        throw std::runtime_error("互斥锁创建错误");
    }
}

bool Mutex::lock() {
    if (pthread_mutex_lock(&mutex) != 0){
        return false;
    }
    return true;
}

bool Mutex::unlock() {
    if (pthread_mutex_unlock(&mutex) != 0){
        return false;
    }
    return true;
}

int Mutex::tryLock() {
    int state = pthread_mutex_trylock(&mutex);
    if (state == EBUSY) {
        return 1;
    }
    if (state != 0){
        return -1;
    }
    return 0;
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex);
}