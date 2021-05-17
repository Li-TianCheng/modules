//
// Created by ltc on 2021/3/9.
//

#ifndef MYPTHREAD_SEMAPHORE_H
#define MYPTHREAD_SEMAPHORE_H

#include <semaphore.h>

class Semaphore {
public:
    Semaphore(int num);
    void wait();
    void post();
    ~Semaphore();
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    void operator=(const Semaphore&) = delete;
    void operator=(Semaphore&&) = delete;
private:
    sem_t semaphore;
};


#endif //MYPTHREAD_SEMAPHORE_H
