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
    void* arg;
    Event(EventKey eventType, void* arg) : eventType(eventType), arg(arg){};
};

class EventSystem {
public:
    EventSystem();
    void registerEvent(EventKey eventType, void (*handleEvent)(void*));
    void unregisterEvent(EventKey eventType);
    void receiveEvent(Event* e);
    void doEvent(Event* e);
    Event* getEvent();
    void cycle();
    static void cycleTask(void* arg);
    virtual void cycleInit();
    virtual void cycleClear();
    virtual ~EventSystem();
    EventSystem(const EventSystem &) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;
private:
    volatile bool shutdown;
    unordered_map<EventKey, void(*)(void*)> map;
    queue<Event*> eventQueue;
    Mutex mutex;
    Condition condition;
};


#endif //EVENTSYSTEM_EVENTSYSTEM_H
