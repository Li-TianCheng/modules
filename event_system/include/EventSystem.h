//
// Created by ltc on 2021/5/14.
//

#ifndef EVENTSYSTEM_EVENTSYSTEM_H
#define EVENTSYSTEM_EVENTSYSTEM_H

#include <unordered_map>
#include <string>
#include <queue>
#include "mem_pool/include/ObjPool.hpp"
#include "my_pthread/include/Condition.h"
#include "EventKey.h"

using std::unordered_map;
using std::string;
using std::queue;

class EventSystem;

struct Event{
    EventKey eventType;
    EventSystem* ptr;
    void* arg;
    Event(EventKey eventType, void* arg, EventSystem* ptr) : eventType(eventType), arg(arg), ptr(ptr){};
};

class EventSystem {
public:
    EventSystem() = default;
    void registerEvent(EventKey eventType, void (*handleEvent)(Event*));
    void unregisterEvent(EventKey eventType);
    void addEvent(Event* e);
    void doEvent(Event* e);
    Event* getEvent();
    void cycle();
    virtual void cycleInit();
    virtual void cycleClear();
    EventSystem(const EventSystem &) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;
private:
    unordered_map<EventKey, void(*)(Event*)> map;
    queue<Event*> eventQueue;
    Mutex mutex;
    Condition condition;
};


#endif //EVENTSYSTEM_EVENTSYSTEM_H
