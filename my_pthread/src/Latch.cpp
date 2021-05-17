//
// Created by ltc on 2021/5/8.
//

#include "Latch.h"

Latch::Latch(int num):count(num), mutex(), condition() {}

void Latch::done() {
    mutex.lock();
    count--;
    condition.notifyAll(mutex);
}

void Latch::wait() {
    if (count == 0){
        return;
    }
    mutex.lock();
    while (count != 0){
        condition.wait(mutex);
    }
    mutex.unlock();
}

