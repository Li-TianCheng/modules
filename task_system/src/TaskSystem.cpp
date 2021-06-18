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
    getThreadPool().receiveEvent(e);
    getThread().join();
}

void TaskSystem::addTask(void (*task)(const shared_ptr<void>&), const shared_ptr<void>& arg) {
    getThreadPool().addTask(task, arg);
}

ThreadPool &TaskSystem::getThreadPool() {
    static ThreadPool threadPool(InitThreadNum, MaxThreadNum, TaskQueueSize);
    return threadPool;
}

Thread &TaskSystem::getThread() {
    static Thread thread;
    return thread;
}

void *TaskSystem::handle(void *) {
    getThreadPool().cycle();
}
