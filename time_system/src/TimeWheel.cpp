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

void TimeWheel::handleTimerEvent(Event* e) {
    if (e->arg == nullptr){
        return;
    }
    Time* time = (Time*)e->arg;
    TimeWheel* t = time->tPtr;
    Event* _e = ObjPool::allocate<Event>(EventTimerTimeOut, time);
    if (time->h != 0){
        t->hour[(t->hIter + time->h) % 24].push(_e);
        return;
    }
    if (time->m != 0){
        t->minute[(t->mIter + time->m) % 60].push(_e);
        return;
    }
    if (time->s != 0){
        t->second[(t->sIter + time->s) % 60].push(_e);
        return;
    }
    if (time->ms != 0){
        t->millisecond[(t->msIter + time->ms) % 1000].push(_e);
        return;
    }
    ObjPool::deallocate(_e);
}

void TimeWheel::handleTickerEvent(Event* e) {
    if (e->arg == nullptr){
        return;
    }
    Time* time = (Time*)e->arg;
    TimeWheel* t = time->tPtr;
    Event* _e = ObjPool::allocate<Event>(EventTickerTimeOut, time);
    if (time->h != 0){
        t->hour[(t->hIter + time->h) % 24].push(_e);
        return;
    }
    if (time->m != 0){
        t->minute[(t->mIter + time->m) % 60].push(_e);
        return;
    }
    if (time->s != 0){
        t->second[(t->sIter + time->s) % 60].push(_e);
        return;
    }
    if (time->ms != 0){
        t->millisecond[(t->msIter + time->ms) % 1000].push(_e);
        return;
    }
    ObjPool::deallocate(_e);
}

void TimeWheel::handleTimerTimeOut(Event *e) {
    Time* t = (Time*)e->arg;
    if (t->ePtr != nullptr){
        Event* _e = ObjPool::allocate<Event>(EventTimerTimeOut, e->arg);
        t->ePtr->receiveEvent(_e);
    }
}

void TimeWheel::handleTickerTimeOut(Event *e) {
    Time* t = (Time*)e->arg;
    TimeWheel* tPtr = t->tPtr;
    if (tPtr->toDelete.find(((Time*)e->arg)->uuid) != tPtr->toDelete.end()){
        tPtr->toDelete.erase(((Time*)e->arg)->uuid);
        return;
    }
    if (t->ePtr != nullptr){
        Event* _e = ObjPool::allocate<Event>(EventTickerTimeOut, e->arg);
        t->ePtr->receiveEvent(_e);
    }
    Event* _e = ObjPool::allocate<Event>(EventTicker, e->arg);
    handleTickerEvent(_e);
}

void TimeWheel::handleTimeOut(Event *e) {
    TimeWheel* tw = (TimeWheel*)e->arg;
    while (!tw->millisecond[tw->msIter].empty()){
        Event* _e = tw->millisecond[tw->msIter].front();
        tw->doEvent(_e);
        tw->millisecond[tw->msIter].pop();
    }
    tw->msIter = (tw->msIter+1) % 1000;
    if (tw->msIter == 0){
        tw->sIter = (tw->sIter+1) % 60;
        while(!tw->second[tw->sIter].empty()){
            Event* _e = tw->second[tw->sIter].front();
            tw->millisecond[((Time*)_e->arg)->ms].push(_e);
            tw->second[tw->sIter].pop();
        }
    }
    if (tw->sIter == 0){
        tw->mIter = (tw->mIter+1) % 60;
        while(!tw->minute[tw->mIter].empty()){
            Event* _e = tw->minute[tw->mIter].front();
            tw->second[((Time*)_e->arg)->s].push(_e);
            tw->minute[tw->mIter].pop();
        }
    }
    if (tw->mIter == 0){
        tw->hIter = (tw->hIter+1) % 24;
        while(!tw->hour[tw->hIter].empty()){
            Event* _e = tw->hour[tw->hIter].front();
            tw->minute[((Time*)_e->arg)->m].push(_e);
            tw->hour[tw->hIter].pop();
        }
    }
}

void TimeWheel::deleteTicker(const string& uuid) {
    toDelete.insert(uuid);
}

