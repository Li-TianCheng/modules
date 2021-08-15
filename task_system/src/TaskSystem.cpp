//
// Created by ltc on 2021/5/14.
//

#include "TaskSystem.h"

void TaskSystem::init() {
    getThreadPool();
    auto time = ConfigSystem::getConfig()["system"]["task_system"]["check_time"];
    ResourceSystem::registerResource(getThreadPool(), time[0].asInt(), time[1].asInt(), time[2].asInt(), time[3].asInt());
}

void TaskSystem::close() {
    ResourceSystem::unregisterResource(getThreadPool());
    getThreadPool()->join();
}

void TaskSystem::addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addTask(task, arg);
}

shared_ptr<ThreadPool> TaskSystem::getThreadPool() {
    static shared_ptr<ThreadPool> threadPool = ObjPool::allocate<ThreadPool>(
            ConfigSystem::getConfig()["system"]["task_system"]["init_thread_num"].asInt(),
            ConfigSystem::getConfig()["system"]["task_system"]["max_thread_num"].asInt(),
            ConfigSystem::getConfig()["system"]["task_system"]["increase_task_num"].asInt());
    return threadPool;
}

void TaskSystem::addPriorityTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addPriorityTask(task, arg);
}
