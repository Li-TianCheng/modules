//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_TASKSYSTEM_H
#define TASKSYSTEM_TASKSYSTEM_H

#include "ThreadPool.h"

static const int InitThreadNum       = 10;
static const int MaxThreadNum        = 30;
static const int TaskQueueSize     = 10;

class TaskSystem {
public:
    static void init();
    static void close();
    static void addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg);
    static void addPriorityTask(void (*task)(shared_ptr<void> arg), shared_ptr<void> arg);
    TaskSystem(const TaskSystem&) = delete;
    TaskSystem(TaskSystem&&) = delete;
    TaskSystem& operator=(const TaskSystem&) = delete;
    TaskSystem& operator=(TaskSystem&&) = delete;
private:
    TaskSystem() = default;
    static shared_ptr<ThreadPool> getThreadPool();
    static Thread& getThread();
    static void* handle(void*);
};


#endif //TASKSYSTEM_TASKSYSTEM_H
