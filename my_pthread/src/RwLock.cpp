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

void RwLock::rdLock() {
    if (pthread_rwlock_rdlock(&rwLock) != 0){
        throw std::runtime_error("读锁加锁错误");
    }
}

void RwLock::wrLock() {
    if (pthread_rwlock_wrlock(&rwLock) != 0){
        throw std::runtime_error("写锁加锁错误");
    }
}

void RwLock::unlock() {
    if (pthread_rwlock_unlock(&rwLock) != 0){
        throw std::runtime_error("读写锁释放错误");
    }
}

bool RwLock::tryRdLock() {
    int state = pthread_rwlock_tryrdlock(&rwLock);
    if (state == EBUSY) {
        return false;
    }
    if (state != 0){
        throw std::runtime_error("读锁尝试错误");
    }
    return true;
}

bool RwLock::tryWrLock() {
    int state = pthread_rwlock_trywrlock(&rwLock);
    if (state == EBUSY) {
        return false;
    }
    if (state != 0){
        throw std::runtime_error("写锁尝试错误");
    }
    return true;
}

RwLock::~RwLock() {
    pthread_rwlock_destroy(&rwLock);
}

