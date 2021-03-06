//
// Created by ltc on 2021/5/14.
//

#ifndef TASKSYSTEM_TASKSYSTEM_H
#define TASKSYSTEM_TASKSYSTEM_H

#include "ThreadPool.h"
#include "resource/include/ResourceSystem.h"
#include "config_system/include/ConfigSystem.h"

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
};


#endif //TASKSYSTEM_TASKSYSTEM_H
