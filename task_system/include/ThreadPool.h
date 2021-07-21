//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_THREADPOOL_H
#define TASKSYSTEM_THREADPOOL_H

#include <iostream>
#include <atomic>
#include <list>
#include <deque>
#include "resource/include/ResourceSystem.h"
#include "my_pthread/include/Condition.h"
#include "my_pthread/include/Thread.h"
#include "time_system/include/TimeSystem.h"
#include "log/include/LogSystem.h"

using std::list;
using std::deque;

class _Thread : public Thread {
public:
    _Thread():Thread(), isBlocking(false){};
    bool isBlocking;
};

class ThreadPool: public Resource {
public:
    ThreadPool(int initNum, int maxNum, int queueSize);
    void addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg);
    void addPriorityTask(void (*task)(shared_ptr<void> arg), shared_ptr<void> arg);
    void close();
    void join();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    ~ThreadPool();
private:
    void increase() override;
    void checkOut() override;
    static void cleanHandler(void* arg);
    static void* taskRoutine(void* arg);
private:
    struct TaskNode{
        void (*task)(shared_ptr<void>);
        shared_ptr<void> arg;
        TaskNode(void (*task)(shared_ptr<void>), shared_ptr<void> arg):task(task), arg(arg){};
    };
    shared_ptr<Time> t;
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
