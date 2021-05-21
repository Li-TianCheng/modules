//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_THREADPOOL_H
#define TASKSYSTEM_THREADPOOL_H

#include <iostream>
#include <atomic>
#include <list>
#include "event_system/include/EventSystem.h"
#include "my_pthread/include/Condition.h"
#include "my_pthread/include/Thread.h"
#include "time_system/include/TimeSystem.h"

static Time CheckTime(0, 0, 1, 0, nullptr);
static const int MaxNum         = 5000;

using std::list;

class _Thread : public Thread {
public:
    _Thread():Thread(), isBlocking(false){};
    bool isBlocking;
};

class ThreadPool: public EventSystem {
public:
    ThreadPool(int initNum, int queueSize);
    void addTask(void (*task)(void*), void* arg);
    void cycleInit() override;
    void cycleClear() override;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    void operator=(const ThreadPool&) = delete;
    void operator=(ThreadPool&&) = delete;
    ~ThreadPool();
private:
    void increasePoolSize();
    void decreasePoolSize();
    void init();
    void close();
    void join();
    static void cleanHandler(void* arg);
    static void* taskRoutine(void* arg);
    static void handleTimeOut(Event* e);
    static void handleIncreasePool(Event* e);
private:
    struct TaskNode{
        void (*task)(void *);
        void* arg;
        TaskNode(void (*task)(void *), void* arg):task(task), arg(arg){};
    };
    string uuid;
    Mutex mutex;
    std::atomic<int> runningNum;
    std::atomic<int> threadNum;
    const int queueSize;
    const int initNum;
    volatile int shutdown;
    Condition condition;
    list<_Thread> threadPool;
    queue<TaskNode> taskQueue;
};

struct Input{
    ThreadPool* ptr;
    _Thread* tPtr;
    Input(ThreadPool* ptr, _Thread* tPtr): ptr(ptr), tPtr(tPtr){};
};

#endif //TASKSYSTEM_THREADPOOL_H
