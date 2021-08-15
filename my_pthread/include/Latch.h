//
// Created by ltc on 2021/5/8.
//

#ifndef MYPTHREAD_LATCH_H
#define MYPTHREAD_LATCH_H

#include <atomic>
#include "Mutex.h"
#include "Condition.h"

class Latch {
public:
    explicit Latch(int num);
    void done();
    void wait();
    Latch(const Latch&) = delete;
    Latch(Latch&&) = delete;
    Latch& operator=(const Latch&) = delete;
    Latch& operator=(Latch&&) = delete;
private:
    std::atomic<int> count;
    Mutex mutex;
    Condition condition;
};


#endif //MYPTHREAD_LATCH_H
