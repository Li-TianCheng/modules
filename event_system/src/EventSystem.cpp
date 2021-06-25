//
// Created by ltc on 2021/5/14.
//

#include "EventSystem.h"

void EventSystem::registerEvent(EventKey eventType, void (*handleEvent)(const shared_ptr<void>&)) {
    map[eventType] = handleEvent;
}

void EventSystem::unregisterEvent(EventKey eventType) {
    map.erase(eventType);
}

void EventSystem::receiveEvent(const shared_ptr<Event>& e) {
    if (shutdown){
        return;
    }
    mutex.lock();
    eventQueue.push(e);
    condition.notify(mutex);
}

void EventSystem::doEvent(const shared_ptr<Event>& e) {
    if (e == nullptr){
        return;
    }
    if (map.find(e->eventType) != map.end()){
        if (map[e->eventType] != nullptr){
            map[e->eventType](e->arg);
        }
    }
}

shared_ptr<Event> EventSystem::getEvent() {
    mutex.lock();
    while (eventQueue.empty()){
        condition.wait(mutex);
    }
    shared_ptr<Event> e = eventQueue.front();
    eventQueue.pop();
    condition.notify(mutex);
    return e;
}

void EventSystem::cycle() {
    cycleInit();
    while (true) {
        shared_ptr<Event> e = getEvent();
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
        shared_ptr<Event> e = eventQueue.front();
        eventQueue.pop();
    }
}

EventSystem::EventSystem() : shutdown(false) {}

void EventSystem::cycleTask(const shared_ptr<void>& arg) {
    (*static_pointer_cast<EventSystem*>(arg))->cycle();
}

void EventSystem::cycleNoBlock(int maxNum) {
    if (shutdown || eventQueue.empty()) {
        return;
    }
    mutex.lock();
    vector<shared_ptr<Event>> temp;
    while (!eventQueue.empty() && (maxNum < 0 || temp.size() < maxNum)) {
        shared_ptr<Event> e = eventQueue.front();
        temp.push_back(e);
        eventQueue.pop();
    }
    mutex.unlock();
    for (auto& e : temp) {
        if (e->eventType == EventEndCycle){
            doEvent(e);
            shutdown = true;
            cycleClear();
            return;
        }
        doEvent(e);
    }
}

