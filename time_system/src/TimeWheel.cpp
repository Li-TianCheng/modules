//
// Created by ltc on 2021/5/14.
//
#include "time_system/include/TimeWheel.h"

TimeWheel::TimeWheel(): millisecond(1000), second(60), minute(60), hour(24),
                        msIter(0), sIter(0), mIter(0), hIter(0) {
    init();
}

void TimeWheel::init() {
    registerEvent(EventTimeOut, handleTimeOut);
    registerEvent(EventTimer, handleTimerEvent);
    registerEvent(EventTicker, handleTickerEvent);
    registerEvent(EventTimerTimeOut, handleTimerTimeOut);
    registerEvent(EventTickerTimeOut, handleTickerTimeOut);
    registerEvent(EventEndCycle, nullptr);
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
    if (tPtr->toDelete.find(t->uuid) != tPtr->toDelete.end()){
        tPtr->toDelete.erase(t->uuid);
        return;
    }
    if (t->ePtr != nullptr){
        Event* e = ObjPool::allocate<Event>(EventTickerTimeOut, t);
        t->ePtr->receiveEvent(e);
    }
    handleTickerEvent(t);
}

void TimeWheel::handleTimeOut(void *arg) {
    TimeWheel* tw = (TimeWheel*)arg;
    while (!tw->millisecond[tw->msIter].empty()){
        Event* e = tw->millisecond[tw->msIter].front();
        tw->doEvent(e);
        tw->millisecond[tw->msIter].pop();
    }
    tw->msIter++;
    if (tw->msIter == 1000){
        tw->msIter = 0;
        tw->sIter = tw->sIter+1;
        if (tw->sIter == 60){
            tw->sIter = 0;
            tw->mIter = tw->mIter+1;
            if (tw->mIter == 60){
                tw->mIter = 0;
                tw->hIter = tw->hIter+1;
                if (tw->hIter == 24){
                    tw->hIter = 0;
                }
                while(!tw->hour[tw->hIter].empty()){
                    Event* e = tw->hour[tw->hIter].front();
                    TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
                    tw->minute[arg->nextTime->m].push(e);
                    tw->hour[tw->hIter].pop();
                }
            }
            while(!tw->minute[tw->mIter].empty()){
                Event* e = tw->minute[tw->mIter].front();
                TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
                tw->second[arg->nextTime->s].push(e);
                tw->minute[tw->mIter].pop();
            }
        }
        while(!tw->second[tw->sIter].empty()){
            Event* e = tw->second[tw->sIter].front();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            tw->millisecond[arg->nextTime->ms].push(e);
            tw->second[tw->sIter].pop();
        }
    }
}

void TimeWheel::deleteTicker(const string& uuid) {
    toDelete.insert(uuid);
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
    for (int i = 0; i < 1000; i++) {
        while (!millisecond[i].empty()){
            Event* e = millisecond[i].front();
            millisecond[i].pop();
            TimeWheelEventArg* arg = (TimeWheelEventArg*)e->arg;
            ObjPool::deallocate(arg->nextTime);
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
            ObjPool::deallocate(arg);
            ObjPool::deallocate(e);
        }
    }
}

