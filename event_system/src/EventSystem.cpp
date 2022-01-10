//
// Created by ltc on 2021/5/14.
//

#include "EventSystem.h"

void EventSystem::registerEvent(EventKey eventType, void (*handleEvent)(shared_ptr<void>)) {
    map[eventType] = handleEvent;
}

void EventSystem::unregisterEvent(EventKey eventType) {
    map.erase(eventType);
}

void EventSystem::receiveEvent(shared_ptr<Event> e) {
    if (shutdown){
        return;
    }
    mutex.lock();
    eventQueue.push_back(e);
    condition.notify(mutex);
}

void EventSystem::receivePriorityEvent(shared_ptr<Event> e) {
	if (shutdown){
		return;
	}
	mutex.lock();
	eventQueue.push_front(e);
	condition.notify(mutex);
}

void EventSystem::doEvent(shared_ptr<Event> e) {
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
    eventQueue.pop_front();
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

void EventSystem::cycleClear() {
    shutdown = true;
    while (!eventQueue.empty()) {
        eventQueue.pop_front();
    }
}

EventSystem::EventSystem() : shutdown(false) {}

void EventSystem::cycleTask(shared_ptr<void> arg) {
    (static_pointer_cast<EventSystem>(arg))->cycle();
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
        eventQueue.pop_front();
    }
    mutex.unlock();
    for (auto e : temp) {
        if (e->eventType == EventEndCycle){
            doEvent(e);
            shutdown = true;
            cycleClear();
            return;
        }
        doEvent(e);
    }
}

void EventSystem::closeCycle() {
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    receiveEvent(e);
}

