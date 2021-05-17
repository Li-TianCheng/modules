//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_TASKSYSTEM_H
#define TASKSYSTEM_TASKSYSTEM_H

#include "ThreadPool.h"

static const int InitNum       = 100;
static const int QueueSize     = 100;

class TaskSystem {
public:
    static void init();
    static void close();
    static void addTask(void (*task)(void*), void* arg);
    TaskSystem(const TaskSystem&) = delete;
    TaskSystem(TaskSystem&&) = delete;
    TaskSystem& operator=(const TaskSystem&) = delete;
    TaskSystem& operator=(TaskSystem&&) = delete;
private:
    TaskSystem() = default;
    static ThreadPool& getThreadPool();
    static Thread& getThread();
    static void* handle(void*);
};


#endif //TASKSYSTEM_TASKSYSTEM_H
