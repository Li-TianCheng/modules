//
// Created by ltc on 2021/3/9.
//

#ifndef MYPTHREAD_BARRIER_H
#define MYPTHREAD_BARRIER_H

#include "Condition.h"

class Barrier {
public:
    explicit Barrier(int threadNum, int cycleNum=-1);
    void wait();
    Barrier(const Barrier&) = delete;
    Barrier(Barrier&&) = delete;
    void operator=(const Barrier&) = delete;
    void operator=(Barrier&&) = delete;
private:
    Mutex mutex;
    Condition condition;
    const int threadNum;
    volatile int cycleNum;
    int counter;
};


#endif //MYPTHREAD_BARRIER_H
