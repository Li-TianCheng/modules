//
// Created by ltc on 2021/5/14.
//

#include "TaskSystem.h"

void TaskSystem::init() {
    getThreadPool();
    ResourceSystem::registerResource(getThreadPool(), 0, 0, 1, 0);
}

void TaskSystem::close() {
    ResourceSystem::unregisterResource(getThreadPool());
    getThreadPool()->join();
}

void TaskSystem::addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addTask(task, arg);
}

shared_ptr<ThreadPool> TaskSystem::getThreadPool() {
    static shared_ptr<ThreadPool> threadPool = ObjPool::allocate<ThreadPool>(InitThreadNum, MaxThreadNum, TaskQueueSize);
    return threadPool;
}

void TaskSystem::addPriorityTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addPriorityTask(task, arg);
}
