//
// Created by ltc on 2021/5/14.
//

#include "EventSystem.h"

void EventSystem::registerEvent(EventKey eventType, void (*handleEvent)(void*)) {
    map[eventType] = handleEvent;
}

void EventSystem::unregisterEvent(EventKey eventType) {
    map.erase(eventType);
}

void EventSystem::receiveEvent(Event* e) {
    if (shutdown){
        return;
    }
    mutex.lock();
    eventQueue.push(e);
    condition.notifyAll(mutex);
}

void EventSystem::doEvent(Event* e) {
    if (e == nullptr){
        return;
    }
    if (map.find(e->eventType) != map.end()){
        if (map[e->eventType] != nullptr){
            map[e->eventType](e->arg);
        }
    }
    ObjPool::deallocate(e);
}

Event* EventSystem::getEvent() {
    mutex.lock();
    while (eventQueue.empty()){
        condition.wait(mutex);
    }
    Event* e = eventQueue.front();
    eventQueue.pop();
    condition.notifyAll(mutex);
    return e;
}

void EventSystem::cycle() {
    cycleInit();
    while (true) {
        Event* e = getEvent();
        if (e->eventType == EventEndCycle){
            doEvent(e);
            break;
        }
        doEvent(e);
    }
    shutdown = true;
    cycleClear();
}

void EventSystem::cycleInit() {}

void EventSystem::cycleClear() {}

EventSystem::~EventSystem() {
    while (!eventQueue.empty()){
        Event* e = eventQueue.front();
        ObjPool::deallocate(e);
        eventQueue.pop();
    }
}

EventSystem::EventSystem() : shutdown(false) {}

