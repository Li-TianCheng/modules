//
// Created by ltc on 2021/5/14.
//
#include "time_system/include/TimeWheel.h"

TimeWheel::TimeWheel(): millisecond(1000), second(60), minute(60), hour(24),
                        msIter(0), sIter(0), mIter(0), hIter(0), shutdown(false) {
    epollFd = epoll_create(1);
    init();
}

void TimeWheel::init() {
    registerEvent(EventTimer, handleTimerEvent);
    registerEvent(EventTicker, handleTickerEvent);
    registerEvent(EventTimerTimeOut, handleTimerTimeOut);
    registerEvent(EventTickerTimeOut, handleTickerTimeOut);
    registerEvent(EventEndCycle, handleEndCycle);
}

void TimeWheel::handleTimerEvent(void* arg) {
    if (arg == nullptr){
        return;
    }
    ((Time*)arg)->tPtr->addTimeToWheel(EventTimerTimeOut, (Time*)arg);
}

void TimeWheel::handleTickerEvent(void* arg) {
    if (arg == nullptr){
        return;
    }
    ((Time*)arg)->tPtr->addTimeToWheel(EventTickerTimeOut, (Time*)arg);
}

void TimeWheel::handleTimerTimeOut(void *arg) {
    ObjPool::deallocate(((TimeWheelEventArg*)arg)->nextTime);
    Time* t = ((TimeWheelEventArg*)arg)->t;
    ObjPool::deallocate((TimeWheelEventArg*)arg);
    if (t->ePtr != nullptr){
        Event* _e = ObjPool::allocate<Event>(EventTimerTimeOut, t);
        t->ePtr->receiveEvent(_e);
    }
}

void TimeWheel::handleTickerTimeOut(void *arg) {
    ObjPool::deallocate(((TimeWheelEventArg*)arg)->nextTime);
    Time* t = ((TimeWheelEventArg*)arg)->t;
    ObjPool::deallocate((TimeWheelEventArg*)arg);
    TimeWheel* tPtr = t->tPtr;
    tPtr->mutex.lock();
    if (tPtr->toDelete.find(t->uuid) != tPtr->toDelete.end()){
        tPtr->toDelete.erase(t->uuid);
        ObjPool::deallocate(t);
        tPtr->mutex.unlock();
        return;
    }
    tPtr->mutex.unlock();
    if (t->ePtr != nullptr){
        Event* e = ObjPool::allocate<Event>(EventTickerTimeOut, t);
        t->ePtr->receiveEvent(e);
    }
    handleTickerEvent(t);
}

void TimeWheel::deleteTicker(const string& uuid) {
    mutex.lock();
    toDelete.insert(uuid);
    mutex.unlock();
}

void TimeWheel::addTimeToWheel(EventKey e, Time *t) {
    Time* nextTime = ObjPool::allocate<Time>(t->h+hIter, t->m+mIter, t->s+sIter, t->ms+msIter, t->ePtr);
    nextTime->s += nextTime->ms / 1000;
    nextTime->m += nextTime->s / 60;
    nextTime->h += nextTime->m / 60;
    nextTime->ms %= 1000;
    nextTime->s %= 60;
    nextTime->m %= 60;
    nextTime->h %= 24;
    TimeWheelEventArg* arg = ObjPool::allocate<TimeWheelEventArg>(t, nextTime);
    Event* _e = ObjPool::allocate<Event>(e, arg);
    if (nextTime->h != hIter){
        hour[nextTime->h].push(_e);
        return;
    }
    if (nextTime->m != mIter){
        minute[nextTime->m].push(_e);
        return;
    }
    if (nextTime->s != sIter){
        second[nextTime->s].push(_e);
        return;
    }
    if (nextTime->ms != msIter){
        millisecond[nextTime->ms].push(_e);
        return;
    }
    ObjPool::deallocate(nextTime);
    ObjPool::deallocate(arg);
    ObjPool::deallocate(_e);
}

TimeWheel::~TimeWheel() {
    mutex.lock();
    for (int i = 0; i < 1000; i++) {
        while (!millisecond[i].empty()){
            Event* e = millisecond[i].front();
            millisecond[i].pop();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            ObjPool::deallocate(arg->nextTime);
            if (toDelete.find(arg->t->uuid) != toDelete.end()){
                toDelete.erase(arg->t->uuid);
                ObjPool::deallocate(arg->t);
            }
            ObjPool::deallocate(arg);
            ObjPool::deallocate(e);
        }
    }
    for (int i = 0; i < 60; i++) {
        while (!second[i].empty()){
            Event* e = second[i].front();
            second[i].pop();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            ObjPool::deallocate(arg->nextTime);
            if (toDelete.find(arg->t->uuid) != toDelete.end()){
                toDelete.erase(arg->t->uuid);
                ObjPool::deallocate(arg->t);
            }
            ObjPool::deallocate(arg);
            ObjPool::deallocate(e);
        }
    }
    for (int i = 0; i < 60; i++) {
        while (!minute[i].empty()){
            Event* e = minute[i].front();
            minute[i].pop();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            ObjPool::deallocate(arg->nextTime);
            if (toDelete.find(arg->t->uuid) != toDelete.end()){
                toDelete.erase(arg->t->uuid);
                ObjPool::deallocate(arg->t);
            }
            ObjPool::deallocate(arg);
            ObjPool::deallocate(e);
        }
    }
    for (int i = 0; i < 24; i++) {
        while (!hour[i].empty()){
            Event* e = hour[i].front();
            hour[i].pop();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            ObjPool::deallocate(arg->nextTime);
            if (toDelete.find(arg->t->uuid) != toDelete.end()){
                toDelete.erase(arg->t->uuid);
                ObjPool::deallocate(arg->t);
            }
            ObjPool::deallocate(arg);
            ObjPool::deallocate(e);
        }
    }
    mutex.unlock();
    ::close(epollFd);
}

void *TimeWheel::timeWheelCycle(void *arg) {
    TimeWheel* t = (TimeWheel*)arg;
    t->cycleInit();
    while (!t->shutdown) {
        t->cycleNoBlock();
        epoll_wait(t->epollFd, nullptr, 1, 1);
        t->update();
    }
}

void TimeWheel::update() {
    while (!millisecond[msIter].empty()){
        Event* e = millisecond[msIter].front();
        doEvent(e);
        millisecond[msIter].pop();
    }
    msIter++;
    if (msIter == 1000){
        msIter = 0;
        sIter = sIter+1;
        if (sIter == 60){
            sIter = 0;
            mIter = mIter+1;
            if (mIter == 60){
                mIter = 0;
                hIter = hIter+1;
                if (hIter == 24){
                    hIter = 0;
                }
                while(!hour[hIter].empty()){
                    Event* e = hour[hIter].front();
                    TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
                    minute[arg->nextTime->m].push(e);
                    hour[hIter].pop();
                }
            }
            while(!minute[mIter].empty()){
                Event* e = minute[mIter].front();
                TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
                second[arg->nextTime->s].push(e);
                minute[mIter].pop();
            }
        }
        while(!second[sIter].empty()){
            Event* e = second[sIter].front();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            millisecond[arg->nextTime->ms].push(e);
            second[sIter].pop();
        }
    }
}

void TimeWheel::handleEndCycle(void *arg) {
    ((TimeWheel*)arg)->shutdown = true;
}

