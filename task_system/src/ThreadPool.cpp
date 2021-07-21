//
// Created by ltc on 2021/5/14.
//

#include "task_system/include/ThreadPool.h"

ThreadPool::ThreadPool(int initNum, int maxNum, int queueSize): initNum(initNum), maxNum(maxNum),
                                                    queueSize(queueSize), threadNum(initNum),
                                                    runningNum(0), shutdown(0), threadPool(initNum) {
    for(auto& thread : threadPool){
        auto arg = new ThreadPoolEventArg(this, &thread);
        thread.run(taskRoutine, arg);
    }
}

void ThreadPool::addTask(void (*task)(shared_ptr<void>), shared_ptr<void> arg) {
    if (shutdown > 0){
        std::cerr << "线程池正在关闭" << std::endl;
        return;
    }
    mutex.lock();
    if (taskQueue.size() >= queueSize) {
        ResourceSystem::receiveEvent(EventIncrease, shared_from_this());
    }
    taskQueue.emplace_back(task, arg);
    condition.notify(mutex);
}

void ThreadPool::addPriorityTask(void (*task)(shared_ptr<void> arg), shared_ptr<void> arg) {
    if (shutdown > 0){
        std::cerr << "线程池正在关闭" << std::endl;
        return;
    }
    mutex.lock();
    taskQueue.emplace_front(task, arg);
    condition.notify(mutex);
}

ThreadPool::~ThreadPool() {
    while(threadNum != 0){}
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
            curr->taskQueue.pop_front();
            curr->condition.notify(curr->mutex);
            curr->runningNum++;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
            t.task(t.arg);
            pthread_setcancelstate(PTHREAD_CANCEL_DEFERRED, nullptr);
            curr->runningNum--;
        }
    pthread_cleanup_pop(0);
}

void ThreadPool::increase() {
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

void ThreadPool::checkOut() {
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
