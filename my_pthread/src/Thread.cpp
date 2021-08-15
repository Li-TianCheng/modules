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

bool Thread::join() {
    if (pthread_join(threadID, &result) != 0){
        return false;
    }
    isRunning = false;
    return true;
}

bool Thread::cancel() {
    if (pthread_cancel(threadID) != 0){
        return false;
    }
    isRunning = false;
    return true;
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

bool Thread::detach() {
    if (pthread_detach(threadID) != 0){
        return false;
    }
    isRunning = false;
    return true;
}
