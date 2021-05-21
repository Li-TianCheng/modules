//
// Created by ltc on 2021/5/14.
//

#include "TimeSystem.h"

void TimeSystem::init() {
    getTimeWheel();
    getTiming().run(timing, nullptr);
    getThread().run(handle, nullptr);
}

void *TimeSystem::handle(void *) {
    getTimeWheel().cycle();
}

void TimeSystem::close() {
    Event* e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    getTimeWheel().receiveEvent(e);
    getTiming().cancel();
    getTiming().join();
    getThread().join();
}

string TimeSystem::receiveEvent(EventKey eventType, Time *arg) {
    arg->tPtr = &getTimeWheel();
    Event* e = ObjPool::allocate<Event>(eventType, arg);
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

void* TimeSystem::timing(void *) {
    int ef = epoll_create(1);
    while (true) {
        epoll_wait(ef, nullptr, 1, 1);
        Event* e = ObjPool::allocate<Event>(EventTimeOut, &getTimeWheel());
        getTimeWheel().receiveEvent(e);
    }
}

Thread &TimeSystem::getTiming() {
    static Thread thread;
    return thread;
}

void TimeSystem::deleteTicker(const string& uuid) {
    getTimeWheel().deleteTicker(uuid);
}

