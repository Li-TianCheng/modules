//
// Created by ltc on 2021/5/14.
//

#include "EventSystem.h"

void EventSystem::registerEvent(EventKey eventType, void (*handleEvent)(Event*)) {
    map[eventType] = handleEvent;
}

void EventSystem::unregisterEvent(EventKey eventType) {
    map.erase(eventType);
}

void EventSystem::receiveEvent(Event* e) {
    mutex.lock();
    eventQueue.push(e);
    condition.notifyAll(mutex);
}

void EventSystem::doEvent(Event* e) {
    if (map.find(e->eventType) != map.end()){
        map[e->eventType](e);
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
        if (e != nullptr){
            if (e->eventType == EventEndCycle){
                break;
            }
            doEvent(e);
        }
    }
    cycleClear();
}

void EventSystem::cycleInit() {}

void EventSystem::cycleClear() {}

