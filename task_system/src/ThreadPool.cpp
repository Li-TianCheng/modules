//
// Created by ltc on 2021/5/14.
//

#include "task_system/include/ThreadPool.h"

ThreadPool::ThreadPool(int initNum, int queueSize): initNum(initNum), queueSize(queueSize), threadNum(initNum),
                                                    runningNum(0), shutdown(0), threadPool(initNum) {
    CheckTime.ePtr = this;
    init();
}

void ThreadPool::addTask(void (*task)(void *), void *arg) {
    if (shutdown > 0){
        std::cerr << "线程池正在关闭" << std::endl;
        return;
    }
    mutex.lock();
    while(shutdown == 0 && taskQueue.size() == queueSize){
        Event* e = ObjPool::allocate<Event>(EventIncreasePool, this);
        receiveEvent(e);
        condition.wait(mutex);
    }
    taskQueue.push(TaskNode(task, arg));
    condition.notifyAll(mutex);
}

void ThreadPool::cycleInit() {
    uuid = TimeSystem::receiveEvent(EventTicker, (Time *) &CheckTime);
    for(auto& thread : threadPool){
        ThreadPoolEventArg* arg = ObjPool::allocate<ThreadPoolEventArg>(this, &thread);
        thread.run(taskRoutine, arg);
    }
}

void ThreadPool::cycleClear() {
    TimeSystem::deleteTicker(uuid);
    join();
}

ThreadPool::~ThreadPool() {
    while(threadNum != 0){}
}

void ThreadPool::init() {
    registerEvent(EventTickerTimeOut, handleTimeOut);
    registerEvent(EventIncreasePool, handleIncreasePool);
    registerEvent(EventEndCycle, nullptr);
}

void ThreadPool::close() {
    shutdown = 2;
    for (auto it = threadPool.begin(); it != threadPool.end();){
        (*it).cancel();
        (*it).detach();
        threadPool.erase(it++);
    }
}

void ThreadPool::join() {
    shutdown = 1;
    for (auto it = threadPool.begin(); it != threadPool.end();){
        (*it).cancel();
        (*it).join();
        threadPool.erase(it++);
    }
}

void ThreadPool::cleanHandler(void *arg) {
    ((ThreadPoolEventArg*)arg)->ptr->mutex.unlock();
    ((ThreadPoolEventArg*)arg)->ptr->threadNum--;
    ObjPool::deallocate((ThreadPoolEventArg*)arg);
}

void *ThreadPool::taskRoutine(void *arg) {
    pthread_cleanup_push(cleanHandler, arg);
        ThreadPool* curr = ((ThreadPoolEventArg*)arg)->ptr;
        while (true) {
            curr->mutex.lock();
            while (curr->shutdown == 2 || curr->taskQueue.empty()){
                ((ThreadPoolEventArg*)arg)->tPtr->isBlocking = true;
                curr->condition.wait(curr->mutex);
            }
            ((ThreadPoolEventArg*)arg)->tPtr->isBlocking = false;
            TaskNode t = curr->taskQueue.front();
            curr->taskQueue.pop();
            curr->condition.notifyAll(curr->mutex);
            curr->runningNum++;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
            t.task(t.arg);
            pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, nullptr);
            curr->runningNum--;
        }
    pthread_cleanup_pop(0);
}

void ThreadPool::handleTimeOut(Event* e) {
    ((ThreadPool*)((Time*)e->arg)->ePtr)->decreasePoolSize();
}

void ThreadPool::increasePoolSize() {
    if (runningNum != threadNum || threadNum >= MaxNum){
        return;
    }
    mutex.lock();
    for (int i = 0; i < initNum; i++){
        threadPool.emplace_back();
        ThreadPoolEventArg* arg = ObjPool::allocate<ThreadPoolEventArg>(this, &threadPool.back());
        threadPool.back().run(taskRoutine, arg);
        threadNum++;
    }
    mutex.unlock();
}

void ThreadPool::decreasePoolSize() {
    if (!taskQueue.empty() || threadNum == initNum ){
        return;
    }
    mutex.lock();
    int target = threadNum-std::max(threadNum-initNum, initNum);
    for (auto it = threadPool.begin(); it != threadPool.end();){
        if (target == 0){
            break;
        }
        if ((*it).isBlocking){
            (*it).cancel();
            (*it).detach();
            threadPool.erase(it++);
            target--;
        }else{
            it++;
        }
    }
    mutex.unlock();
}

void ThreadPool::handleIncreasePool(Event *e) {
    ((ThreadPool*)e->arg)->increasePoolSize();
}
