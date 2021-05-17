//
// Created by ltc on 2021/3/7.
//

#include <stdexcept>
#include "Thread.h"

void Thread::run(void *(*handle)(void *), void* arg) {
    if (pthread_create(&threadID, nullptr, handle, arg) != 0){
        throw std::runtime_error("线程创建错误");
    }
    isRunning = true;
}

void Thread::join() {
    if (pthread_join(threadID, &result) != 0){
        throw std::runtime_error("线程连接错误");
    }
    isRunning = false;
}

void Thread::cancel() {
    if (pthread_cancel(threadID) != 0){
        throw std::runtime_error("线程取消错误");
    }
    isRunning = false;
}

pthread_t Thread::getID() const {
    return threadID;
}

bool Thread::getState() const {
    return isRunning;
}

void* Thread::getResult() const {
    return result;
}

void Thread::detach() {
    if (pthread_detach(threadID) != 0){
        throw std::runtime_error("线程分离错误");
    }
    isRunning = false;
}
