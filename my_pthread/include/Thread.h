//
// Created by ltc on 2021/3/7.
//

#ifndef MYPTHREAD_THREAD_H
#define MYPTHREAD_THREAD_H

#include <pthread.h>

class Thread {
public:
    Thread() = default;
    void run(void* handle(void*), void* arg);
    void join();
    void detach();
    void cancel();
    pthread_t getID() const;
    bool getState() const;
    void* getResult() const;
    Thread(const Thread&) = delete;
    Thread(Thread&&) = delete;
    void operator=(const Thread&) = delete;
    void operator=(Thread&&) = delete;
private:
    bool isRunning;
    pthread_t threadID;
    void* result;
};


#endif //MYPTHREAD_THREAD_H
