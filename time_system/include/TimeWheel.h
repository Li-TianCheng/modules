//
// Created by ltc on 2021/5/14.
//

#ifndef TIMESYSTEM_TIMEWHEEL_H
#define TIMESYSTEM_TIMEWHEEL_H

#include <vector>
#include <sys/time.h>
#include "event_system/include/EventSystem.h"

using std::vector;

class TimeWheel;

struct Time{
    int ms;
    int s;
    int m;
    int h;
    TimeWheel* tPtr = nullptr;
    Time(int h, int m, int s, int ms): ms(ms), s(s), m(m), h(h){};
};

class TimeWheel: public EventSystem {
public:
    TimeWheel();
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel(TimeWheel&&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;
    TimeWheel& operator=(TimeWheel&&) = delete;
private:
    void init();
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
