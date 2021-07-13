//
// Created by ltc on 2021/5/14.
//

#include "TimeSystem.h"

void TimeSystem::init() {
    getTimeWheel();
    getThread().run(handle, nullptr);
}

void TimeSystem::close() {
    auto e = ObjPool::allocate<Event>(EventEndCycle, getTimeWheel());
    getTimeWheel()->receiveEvent(e);
    getThread().join();
}

void TimeSystem::receiveEvent(EventKey eventType, shared_ptr<Time> arg) {
    arg->tPtr = getTimeWheel();
    auto e = ObjPool::allocate<Event>(eventType, arg);
    getTimeWheel()->receiveEvent(e);
}

shared_ptr<TimeWheel> TimeSystem::getTimeWheel() {
    static shared_ptr<TimeWheel> timeWheel = ObjPool::allocate<TimeWheel>();
    return timeWheel;
}

Thread &TimeSystem::getThread() {
    static Thread thread;
    return thread;
}

void TimeSystem::deleteTicker(shared_ptr<Time> arg) {
    auto e = ObjPool::allocate<Event>(EventDeleteTicker, arg);
    getTimeWheel()->receiveEvent(e);
}

void *TimeSystem::handle(void *) {
    getTimeWheel()->timeWheelCycle();
}

