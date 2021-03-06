//
// Created by ltc on 2021/3/7.
//

#ifndef MYPTHREAD_MUTEX_H
#define MYPTHREAD_MUTEX_H

#include <pthread.h>

class Mutex {
public:
    Mutex();
    bool lock();
    bool unlock();
    int tryLock();
    ~Mutex();
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex& operator=(Mutex&&) = delete;
private:
    friend class Condition;
    pthread_mutex_t mutex;
};


#endif //MYPTHREAD_MUTEX_H
