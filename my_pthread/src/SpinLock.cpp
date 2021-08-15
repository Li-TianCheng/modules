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

bool SpinLock::lock() {
    if (pthread_spin_lock(&spinLock) != 0){
        return false;
    }
    return true;
}

bool SpinLock::unlock() {
    if (pthread_spin_unlock(&spinLock) != 0){
        return false;
    }
    return true;
}

int SpinLock::tryLock() {
    int state = pthread_spin_trylock(&spinLock);
    if (state == EBUSY) {
        return 1;
    }
    if (state != 0){
        return -1;
    }
    return 0;
}

SpinLock::~SpinLock() {
    pthread_spin_destroy(&spinLock);
}


