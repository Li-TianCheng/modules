//
// Created by ltc on 2021/5/14.
//

#include "task_system/include/ThreadPool.h"

ThreadPool::ThreadPool(int initNum, int maxNum, int queueSize): initNum(initNum), maxNum(maxNum),
                                                    queueSize(queueSize), threadNum(initNum),
                                                    runningNum(0), shutdown(0), threadPool(initNum) {
    init();
}

void ThreadPool::addTask(void (*task)(const shared_ptr<void>&), const shared_ptr<void>&arg) {
    if (shutdown > 0){
        std::cerr << "线程池正在关闭" << std::endl;
        return;
    }
    mutex.lock();
    while(shutdown == 0 && taskQueue.size() == queueSize){
        auto _arg = ObjPool::allocate<ThreadPool*>(this);
        auto e = ObjPool::allocate<Event>(EventIncreasePool, _arg);
        receiveEvent(e);
        condition.wait(mutex);
    }
    taskQueue.push(TaskNode(task, arg));
    condition.notifyAll(mutex);
}

void ThreadPool::cycleInit() {
    auto t = ObjPool::allocate<Time>(0, 0, 1, 0, this);
    uuid = TimeSystem::receiveEvent(EventTicker, t);
    for(auto& thread : threadPool){
        auto arg = new ThreadPoolEventArg(this, &thread);
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
    delete (ThreadPoolEventArg*)arg;
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

void ThreadPool::handleTimeOut(const shared_ptr<void>& arg) {
    ((ThreadPool*)(static_pointer_cast<Time>(arg))->ePtr)->decreasePoolSize();
}

void ThreadPool::increasePoolSize() {
    if (runningNum != threadNum || threadNum >= maxNum){
        return;
    }
    mutex.lock();
    for (int i = 0; i < initNum/3; i++){
        threadPool.emplace_back();
        ThreadPoolEventArg* arg = new ThreadPoolEventArg(this, &threadPool.back());
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
    int target = threadNum-std::max(threadNum-initNum/3, initNum);
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

void ThreadPool::handleIncreasePool(const shared_ptr<void>& arg) {
    (*static_pointer_cast<ThreadPool*>(arg))->increasePoolSize();
}
