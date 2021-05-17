//
// Created by ltc on 2021/3/9.
//

#ifndef MYPTHREAD_SPINLOCK_H
#define MYPTHREAD_SPINLOCK_H

#include <pthread.h>

class SpinLock {
public:
    SpinLock();
    void lock();
    void unlock();
    bool tryLock();
    ~SpinLock();
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    void operator=(const SpinLock&) = delete;
    void operator=(SpinLock&&) = delete;
private:
    pthread_spinlock_t spinLock;
};


#endif //MYPTHREAD_SPINLOCK_H
