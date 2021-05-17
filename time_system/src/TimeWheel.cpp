//
// Created by ltc on 2021/5/14.
//
#include <iostream>
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
    Time* input = (Time*)e->arg;
    TimeWheel* t = input->tPtr;
    Event* _e = ObjPool::allocate<Event>(EventTimerTimeOut, input, e->ptr);
    if (input->h != 0){
        t->hour[(t->hIter+input->h)%24].push(_e);
        return;
    }
    if (input->m != 0){
        t->minute[(t->mIter+input->m)%60].push(_e);
        return;
    }
    if (input->s != 0){
        t->second[(t->sIter+input->s)%60].push(_e);
        return;
    }
    if (input->ms != 0){
        t->millisecond[(t->msIter+input->ms)%1000].push(_e);
        return;
    }
    ObjPool::deallocate(_e);
}

void TimeWheel::handleTickerEvent(Event* e) {
    if (e->arg == nullptr){
        return;
    }
    Time* input = (Time*)e->arg;
    TimeWheel* t = input->tPtr;
    Event* _e = ObjPool::allocate<Event>(EventTickerTimeOut, input, e->ptr);
    if (input->h != 0){
        t->hour[(t->hIter+input->h)%24].push(_e);
        return;
    }
    if (input->m != 0){
        t->minute[(t->mIter+input->m)%60].push(_e);
        return;
    }
    if (input->s != 0){
        t->second[(t->sIter+input->s)%60].push(_e);
        return;
    }
    if (input->ms != 0){
        t->millisecond[(t->msIter+input->ms)%1000].push(_e);
        return;
    }
    ObjPool::deallocate(_e);
}

void TimeWheel::handleTimerTimeOut(Event *e) {
    if (e->ptr != nullptr){
        Event* _e = ObjPool::allocate<Event>(EventTimerTimeOut, e->ptr, ((Time*)e)->tPtr);
        e->ptr->receiveEvent(_e);
    }
}

void TimeWheel::handleTickerTimeOut(Event *e) {
    if (e->ptr != nullptr){
        Event* _e = ObjPool::allocate<Event>(EventTickerTimeOut, e->ptr, ((Time*)e)->tPtr);
        e->ptr->receiveEvent(_e);
    }
    Event* _e = ObjPool::allocate<Event>(EventTicker, e->arg, e->ptr);
    handleTickerEvent(_e);
}

void TimeWheel::handleTimeOut(Event *e) {
    TimeWheel* tw = (TimeWheel*)e->arg;
    while (!tw->millisecond[tw->msIter].empty()){
        Event* e = tw->millisecond[tw->msIter].front();
        tw->doEvent(e);
        tw->millisecond[tw->msIter].pop();
    }
    tw->msIter = (tw->msIter+1) % 1000;
    if (tw->msIter == 0){
        tw->sIter = (tw->sIter+1) % 60;
        while(!tw->second[tw->sIter].empty()){
            Event* e = tw->second[tw->sIter].front();
            tw->millisecond[((Time*)e->arg)->ms].push(e);
            tw->second[tw->sIter].pop();
        }
    }
    if (tw->sIter == 0){
        tw->mIter = (tw->mIter+1) % 60;
        while(!tw->minute[tw->mIter].empty()){
            Event* e = tw->minute[tw->mIter].front();
            tw->second[((Time*)e->arg)->s].push(e);
            tw->minute[tw->mIter].pop();
        }
    }
    if (tw->mIter == 0){
        tw->hIter = (tw->hIter+1) % 24;
        while(!tw->hour[tw->hIter].empty()){
            Event* e = tw->hour[tw->hIter].front();
            tw->minute[((Time*)e->arg)->m].push(e);
            tw->hour[tw->hIter].pop();
        }
    }
}

