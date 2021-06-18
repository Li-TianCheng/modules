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

string TimeSystem::receiveEvent(EventKey eventType, const shared_ptr<Time>& arg) {
    arg->tPtr = &getTimeWheel();
    auto e = ObjPool::allocate<Event>(eventType, arg);
    getTimeWheel().receiveEvent(e);
    return arg->uuid;
}

TimeWheel &TimeSystem::getTimeWheel() {
    static TimeWheel timeWheel;
    return timeWheel;
}

Thread &TimeSystem::getThread() {
    static Thread thread;
    return thread;
}

void TimeSystem::deleteTicker(const string& uuid) {
    getTimeWheel().deleteTicker(uuid);
}

