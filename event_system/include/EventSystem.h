//
// Created by ltc on 2021/5/14.
//

#ifndef EVENTSYSTEM_EVENTSYSTEM_H
#define EVENTSYSTEM_EVENTSYSTEM_H

#include <unordered_map>
#include <string>
#include <deque>
#include <atomic>
#include "mem_pool/include/ObjPool.hpp"
#include "my_pthread/include/Condition.h"
#include "EventKey.h"

using std::unordered_map;
using std::string;
using std::deque;

class EventSystem;

struct Event{
    EventKey eventType;
    shared_ptr<void> arg;
    Event(EventKey eventType, shared_ptr<void> arg) : eventType(eventType), arg(arg){};
};

class EventSystem: public std::enable_shared_from_this<EventSystem> {
public:
    EventSystem();
	void closeCycle();
    virtual ~EventSystem() = default;
    EventSystem(const EventSystem &) = delete;
    EventSystem(EventSystem&&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    EventSystem& operator=(EventSystem&&) = delete;
protected:
	void registerEvent(EventKey eventType, void (*handleEvent)(shared_ptr<void>));
	void unregisterEvent(EventKey eventType);
	void receiveEvent(shared_ptr<Event> e);
	void receivePriorityEvent(shared_ptr<Event> e);
	void doEvent(shared_ptr<Event> e);
	shared_ptr<Event> getEvent();
	void cycle();
	void cycleNoBlock(int maxNum);
	static void cycleTask(shared_ptr<void> arg);
	virtual void cycleInit();
	virtual void cycleClear();
private:
	friend class TimeWheel;
    std::atomic<bool> shutdown;
    unordered_map<EventKey, void(*)(shared_ptr<void> )> map;
    deque<shared_ptr<Event>> eventQueue;
    Mutex mutex;
    Condition condition;
};


#endif //EVENTSYSTEM_EVENTSYSTEM_H
