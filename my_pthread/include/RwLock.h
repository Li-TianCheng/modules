//
// Created by ltc on 2021/3/9.
//

#ifndef MYPTHREAD_RWLOCK_H
#define MYPTHREAD_RWLOCK_H

#include <pthread.h>

class RwLock {
public:
    RwLock();
    void rdLock();
    void wrLock();
    void unlock();
    bool tryRdLock();
    bool tryWrLock();
    ~RwLock();
    RwLock(const RwLock&) = delete;
    RwLock(RwLock&&) = delete;
    void operator=(const RwLock&) = delete;
    void operator=(RwLock&&) = delete;
private:
    pthread_rwlock_t  rwLock;
};


#endif //MYPTHREAD_RWLOCK_H
