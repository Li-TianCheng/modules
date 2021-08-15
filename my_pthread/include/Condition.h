//
// Created by ltc on 2021/3/7.
//

#ifndef MYPTHREAD_CONDITION_H
#define MYPTHREAD_CONDITION_H

#include "Mutex.h"

class Condition {
public:
    Condition();
    bool wait(Mutex& mutex);
    int wait(Mutex& mutex, const unsigned int timeout);
    bool notify(Mutex& mutex);
    bool notifyAll(Mutex& mutex);
    ~Condition();
    Condition(const Condition&) = delete;
    Condition(Condition&&) = delete;
    Condition& operator=(const Condition&) = delete;
    Condition& operator=(Condition&&) = delete;
private:
    pthread_cond_t condition;
};


#endif //MYPTHREAD_CONDITION_H
