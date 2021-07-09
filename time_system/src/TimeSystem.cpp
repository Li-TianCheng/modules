//
// Created by ltc on 2021/5/14.
//

#include "TimeSystem.h"

void TimeSystem::init() {
    getTimeWheel();
    getThread().run(getTimeWheel().timeWheelCycle, &getTimeWheel());
}

void TimeSystem::close() {
    auto arg = ObjPool::allocate<TimeWheel*>(&getTimeWheel());
    auto e = ObjPool::allocate<Event>(EventEndCycle, arg);
    getTimeWheel().receiveEvent(e);
    getThread().join();
}

void TimeSystem::receiveEvent(EventKey eventType, shared_ptr<Time> arg) {
    arg->tPtr = &getTimeWheel();
    auto e = ObjPool::allocate<Event>(eventType, arg);
    getTimeWheel().receiveEvent(e);
}

TimeWheel &TimeSystem::getTimeWheel() {
    static TimeWheel timeWheel;
    return timeWheel;
}

Thread &TimeSystem::getThread() {
    static Thread thread;
    return thread;
}

void TimeSystem::deleteTicker(shared_ptr<Time> arg) {
    auto e = ObjPool::allocate<Event>(EventDeleteTicker, arg);
    getTimeWheel().receiveEvent(e);
}

