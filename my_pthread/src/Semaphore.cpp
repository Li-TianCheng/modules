//
// Created by ltc on 2021/3/9.
//

#include "Semaphore.h"
#include <stdexcept>

Semaphore::Semaphore(int num):semaphore(){
    if (sem_init(&semaphore, 0, num) != 0){
        throw std::runtime_error("信号量创建错误");
    }
}

void Semaphore::wait() {
    if (sem_wait(&semaphore) != 0){
        throw std::runtime_error("信号量等待错误");
    }
}

void Semaphore::post() {
    if (sem_post(&semaphore) != 0){
        throw std::runtime_error("信号量释放错误");
    }
}

Semaphore::~Semaphore() {
    sem_destroy(&semaphore);
}