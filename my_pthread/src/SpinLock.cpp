//
// Created by ltc on 2021/3/9.
//

#include "SpinLock.h"
#include <stdexcept>


SpinLock::SpinLock():spinLock() {
    if (pthread_spin_init(&spinLock, PTHREAD_PROCESS_SHARED) != 0){
        throw std::runtime_error("自旋锁创建错误");
    }
}

void SpinLock::lock() {
    if (pthread_spin_lock(&spinLock) != 0){
        throw std::runtime_error("自旋锁加锁错误");
    }
}

void SpinLock::unlock() {
    if (pthread_spin_unlock(&spinLock) != 0){
        throw std::runtime_error("自旋锁释放锁错误");
    }
}

bool SpinLock::tryLock() {
    int state = pthread_spin_trylock(&spinLock);
    if (state == EBUSY) {
        return false;
    }
    if (state != 0){
        throw std::runtime_error("自旋锁尝试错误");
    }
    return true;
}

SpinLock::~SpinLock() {
    pthread_spin_destroy(&spinLock);
}


