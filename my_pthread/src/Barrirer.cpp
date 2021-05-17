//
// Created by ltc on 2021/3/9.
//

#include "Barrier.h"

Barrier::Barrier(int threadNum, int cycleNum):mutex(), condition(),
        threadNum(threadNum), cycleNum(cycleNum), counter(0){}

void Barrier::wait(){
    if (cycleNum == 0){
        return;
    }
    mutex.lock();
    counter++;
    if (counter == threadNum){
        counter = 0;
        if (cycleNum != -1){
            cycleNum--;
        }
        condition.notifyAll(mutex);
    }else{
        int cancel = 0;
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancel);
        condition.wait(mutex);
        pthread_setcancelstate(cancel, nullptr);
        mutex.unlock();
    }
}