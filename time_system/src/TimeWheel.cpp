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
    registerEvent(EventDeleteTicker, handleDeleteTicker);
    registerEvent(EventEndCycle, handleEndCycle);
}

void TimeWheel::handleTimerEvent(shared_ptr<void> arg) {
    if (arg == nullptr){
        return;
    }
    static_pointer_cast<Time>(arg)->tPtr.lock()->addTimeToWheel(EventTimerTimeOut, static_pointer_cast<Time>(arg));
}

void TimeWheel::handleTickerEvent(shared_ptr<void> arg) {
    if (arg == nullptr){
        return;
    }
    static_pointer_cast<Time>(arg)->tPtr.lock()->addTimeToWheel(EventTickerTimeOut, static_pointer_cast<Time>(arg));
}

void TimeWheel::handleTimerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<TimeWheelEventArg>(arg)->t;
    if (t->ePtr.lock() != nullptr){
        auto e = ObjPool::allocate<Event>(EventTimerTimeOut, t);
        t->ePtr.lock()->receiveEvent(e);
    }
}

void TimeWheel::handleTickerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<TimeWheelEventArg>(arg)->t;
    auto tPtr = t->tPtr;
    if (tPtr.lock()->toDelete.find(t) != tPtr.lock()->toDelete.end()){
        tPtr.lock()->toDelete.erase(t);
        return;
    }
    if (t->ePtr.lock() != nullptr){
        auto e = ObjPool::allocate<Event>(EventTickerTimeOut, t);
        t->ePtr.lock()->receiveEvent(e);
    }
    handleTickerEvent(t);
}

void TimeWheel::handleDeleteTicker(shared_ptr<void> arg) {
    if (arg == nullptr) {
        return;
    }
    auto t = static_pointer_cast<Time>(arg);
    t->ePtr.lock() = nullptr;
    t->tPtr.lock()->toDelete.insert(t);
}

void TimeWheel::addTimeToWheel(EventKey e, shared_ptr<Time> t) {
    auto nextTime = ObjPool::allocate<Time>(t->h+hIter, t->m+mIter, t->s+sIter, t->ms+msIter, t->ePtr.lock());
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

void TimeWheel::timeWheelCycle() {
    cycleInit();
    while (!shutdown) {
        cycleNoBlock(-1);
        epoll_wait(epollFd, nullptr, 1, 1);
        update();
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

void TimeWheel::handleEndCycle(shared_ptr<void> arg) {
    (static_pointer_cast<TimeWheel>(arg))->shutdown = true;
}

