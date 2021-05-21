//
// Created by ltc on 2021/5/14.
//

#ifndef TIMESYSTEM_TIMEWHEEL_H
#define TIMESYSTEM_TIMEWHEEL_H

#include <vector>
#include <uuid/uuid.h>
#include <unordered_set>
#include "event_system/include/EventSystem.h"

using std::vector;
using std::unordered_set;

class TimeWheel;

struct Time{
    int ms;
    int s;
    int m;
    int h;
    string uuid;
    EventSystem* ePtr;
    TimeWheel* tPtr = nullptr;
    Time(int h, int m, int s, int ms, EventSystem* ePtr): ms(ms), s(s), m(m), h(h), ePtr(ePtr){
        uuid_t uu;
        uuid_generate_time(uu);
        uuid = (char*)uu;
    };
};

class TimeWheel: public EventSystem {
public:
    TimeWheel();
    void deleteTicker(const string& uuid);
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel(TimeWheel&&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;
    TimeWheel& operator=(TimeWheel&&) = delete;
private:
    void init();
    unordered_set<string> toDelete;
    vector<queue<Event*>> millisecond;
    vector<queue<Event*>> second;
    vector<queue<Event*>> minute;
    vector<queue<Event*>> hour;
    int msIter;
    int sIter;
    int mIter;
    int hIter;
    static void handleTimerEvent(Event* e);
    static void handleTickerEvent(Event* e);
    static void handleTimerTimeOut(Event* e);
    static void handleTickerTimeOut(Event* e);
    static void handleTimeOut(Event* e);
};

#endif //TIMESYSTEM_TIMEWHEEL_H
