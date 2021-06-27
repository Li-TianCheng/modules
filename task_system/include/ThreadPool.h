//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_THREADPOOL_H
#define TASKSYSTEM_THREADPOOL_H

#include <iostream>
#include <atomic>
#include <list>
#include <deque>
#include "event_system/include/EventSystem.h"
#include "my_pthread/include/Condition.h"
#include "my_pthread/include/Thread.h"
#include "time_system/include/TimeSystem.h"

using std::list;
using std::deque;

class _Thread : public Thread {
public:
    _Thread():Thread(), isBlocking(false){};
    bool isBlocking;
};

class ThreadPool: public EventSystem {
public:
    ThreadPool(int initNum, int maxNum, int queueSize);
    void addTask(void (*task)(const shared_ptr<void>&), const shared_ptr<void>&arg);
    void addPriorityTask(void (*task)(const shared_ptr<void>& arg), const shared_ptr<void>&arg);
    void cycleInit() override;
    void cycleClear() override;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    ~ThreadPool() override;
private:
    void increasePoolSize();
    void decreasePoolSize();
    void init();
    void close();
    void join();
    static void cleanHandler(void* arg);
    static void* taskRoutine(void* arg);
    static void handleTimeOut(const shared_ptr<void>& arg);
    static void handleIncreasePool(const shared_ptr<void>& arg);
private:
    struct TaskNode{
        void (*task)(const shared_ptr<void>&);
        shared_ptr<void> arg;
        TaskNode(void (*task)(const shared_ptr<void>&), const shared_ptr<void>& arg):task(task), arg(arg){};
    };
    string uuid;
    Mutex mutex;
    std::atomic<int> runningNum;
    std::atomic<int> threadNum;
    const int queueSize;
    const int initNum;
    const int maxNum;
    std::atomic<int> shutdown;
    Condition condition;
    list<_Thread> threadPool;
    deque<TaskNode> taskQueue;
};

struct ThreadPoolEventArg{
    ThreadPool* ptr;
    _Thread* tPtr;
    ThreadPoolEventArg(ThreadPool* ptr, _Thread* tPtr): ptr(ptr), tPtr(tPtr){};
};

#endif //TASKSYSTEM_THREADPOOL_H
