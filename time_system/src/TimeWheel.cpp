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

void TimeWheel::handleTimerEvent(const shared_ptr<void>& arg) {
    if (arg == nullptr){
        return;
    }
    static_pointer_cast<Time>(arg)->tPtr->addTimeToWheel(EventTimerTimeOut, static_pointer_cast<Time>(arg));
}

void TimeWheel::handleTickerEvent(const shared_ptr<void>& arg) {
    if (arg == nullptr){
        return;
    }
    static_pointer_cast<Time>(arg)->tPtr->addTimeToWheel(EventTickerTimeOut, static_pointer_cast<Time>(arg));
}

void TimeWheel::handleTimerTimeOut(const shared_ptr<void>& arg) {
    auto t = static_pointer_cast<TimeWheelEventArg>(arg)->t;
    if (t->ePtr != nullptr){
        auto _e = ObjPool::allocate<Event>(EventTimerTimeOut, t);
        t->ePtr->receiveEvent(_e);
    }
}

void TimeWheel::handleTickerTimeOut(const shared_ptr<void>& arg) {
    auto t = static_pointer_cast<TimeWheelEventArg>(arg)->t;
    TimeWheel* tPtr = t->tPtr;
    tPtr->mutex.lock();
    if (tPtr->toDelete.find(t->uuid) != tPtr->toDelete.end()){
        tPtr->toDelete.erase(t->uuid);
        tPtr->mutex.unlock();
        return;
    }
    tPtr->mutex.unlock();
    if (t->ePtr != nullptr){
        auto e = ObjPool::allocate<Event>(EventTickerTimeOut, t);
        t->ePtr->receiveEvent(e);
    }
    handleTickerEvent(t);
}

void TimeWheel::deleteTicker(const string& uuid) {
    mutex.lock();
    toDelete.insert(uuid);
    mutex.unlock();
}

void TimeWheel::addTimeToWheel(EventKey e, const shared_ptr<Time>& t) {
    auto nextTime = ObjPool::allocate<Time>(t->h+hIter, t->m+mIter, t->s+sIter, t->ms+msIter, t->ePtr);
    nextTime->s += nextTime->ms / 1000;
    nextTime->m += nextTime->s / 60;
    nextTime->h += nextTime->m / 60;
    nextTime->ms %= 1000;
    nextTime->s %= 60;
    nextTime->m %= 60;
    nextTime->h %= 24;
    auto arg = ObjPool::allocate<TimeWheelEventArg>(t, nextTime);
    auto _e = ObjPool::allocate<Event>(e, arg);
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
}

TimeWheel::~TimeWheel() {
    ::close(epollFd);
}

void *TimeWheel::timeWheelCycle(void *arg) {
    TimeWheel* t = (TimeWheel*)arg;
    t->cycleInit();
    while (!t->shutdown) {
        t->cycleNoBlock(-1);
        epoll_wait(t->epollFd, nullptr, 1, 1);
        t->update();
    }
}

void TimeWheel::update() {
    while (!millisecond[msIter].empty()){
        auto e = millisecond[msIter].front();
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
                    auto e = hour[hIter].front();
                    auto arg = static_pointer_cast<TimeWheelEventArg>(e->arg);
                    minute[arg->nextTime->m].push(e);
                    hour[hIter].pop();
                }
            }
            while(!minute[mIter].empty()){
                auto e = minute[mIter].front();
                auto arg = static_pointer_cast<TimeWheelEventArg>(e->arg);
                second[arg->nextTime->s].push(e);
                minute[mIter].pop();
            }
        }
        while(!second[sIter].empty()){
            auto e = second[sIter].front();
            auto arg = static_pointer_cast<TimeWheelEventArg>(e->arg);
            millisecond[arg->nextTime->ms].push(e);
            second[sIter].pop();
        }
    }
}

void TimeWheel::handleEndCycle(const shared_ptr<void>& arg) {
    (*static_pointer_cast<TimeWheel*>(arg))->shutdown = true;
}

