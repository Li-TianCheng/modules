//
// Created by ltc on 2021/3/9.
//

#include "RwLock.h"
#include <stdexcept>


RwLock::RwLock():rwLock(){
    if (pthread_rwlock_init(&rwLock, nullptr) != 0){
        throw std::runtime_error("读写锁创建错误");
    }
}

bool RwLock::rdLock() {
    if (pthread_rwlock_rdlock(&rwLock) != 0){
        return false;
    }
    return true;
}

bool RwLock::wrLock() {
    if (pthread_rwlock_wrlock(&rwLock) != 0){
        return false;
    }
    return true;
}

bool RwLock::unlock() {
    if (pthread_rwlock_unlock(&rwLock) != 0){
        return false;
    }
    return true;
}

int RwLock::tryRdLock() {
    int state = pthread_rwlock_tryrdlock(&rwLock);
    if (state == EBUSY) {
        return 1;
    }
    if (state != 0){
        return -1;
    }
    return 0;
}

int RwLock::tryWrLock() {
    int state = pthread_rwlock_trywrlock(&rwLock);
    if (state == EBUSY) {
        return 1;
    }
    if (state != 0){
        return -1;
    }
    return 0;
}

RwLock::~RwLock() {
    pthread_rwlock_destroy(&rwLock);
}

