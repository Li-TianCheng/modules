//
// Created by ltc on 2021/3/9.
//

#ifndef MYPTHREAD_BARRIER_H
#define MYPTHREAD_BARRIER_H

#include <atomic>
#include "Condition.h"

class Barrier {
public:
    explicit Barrier(int threadNum, int cycleNum=-1);
    void wait();
    Barrier(const Barrier&) = delete;
    Barrier(Barrier&&) = delete;
    Barrier& operator=(const Barrier&) = delete;
    Barrier& operator=(Barrier&&) = delete;
private:
    Mutex mutex;
    Condition condition;
    const int threadNum;
    std::atomic<int> cycleNum;
    int counter;
};


#endif //MYPTHREAD_BARRIER_H
