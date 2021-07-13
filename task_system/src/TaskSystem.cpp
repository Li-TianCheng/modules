//
// Created by ltc on 2021/5/14.
//

#include "TaskSystem.h"

void TaskSystem::init() {
    getThreadPool();
    getThread().run(handle, nullptr);
}

void TaskSystem::close() {
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    getThreadPool()->receiveEvent(e);
    getThread().join();
}

void TaskSystem::addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addTask(task, arg);
}

shared_ptr<ThreadPool> TaskSystem::getThreadPool() {
    static shared_ptr<ThreadPool> threadPool = ObjPool::allocate<ThreadPool>(InitThreadNum, MaxThreadNum, TaskQueueSize);
    return threadPool;
}

Thread &TaskSystem::getThread() {
    static Thread thread;
    return thread;
}

void *TaskSystem::handle(void *) {
    getThreadPool()->cycle();
}

void TaskSystem::addPriorityTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    getThreadPool()->addPriorityTask(task, arg);
}
