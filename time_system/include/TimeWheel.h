//
// Created by ltc on 2021/5/14.
//

#ifndef TIMESYSTEM_TIMEWHEEL_H
#define TIMESYSTEM_TIMEWHEEL_H

#include <vector>
#include <unordered_set>
#include "event_system/include/EventSystem.h"
#include "Time.h"

using std::vector;
using std::unordered_set;

class TimeWheel: public EventSystem {
public:
    TimeWheel();
    ~TimeWheel() override;
    void deleteTicker(const string& uuid);
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel(TimeWheel&&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;
    TimeWheel& operator=(TimeWheel&&) = delete;
private:
    void init();
    void addTimeToWheel(EventKey e, Time* t);
    unordered_set<string> toDelete;
    vector<queue<Event*>> millisecond;
    vector<queue<Event*>> second;
    vector<queue<Event*>> minute;
    vector<queue<Event*>> hour;
    int msIter;
    int sIter;
    int mIter;
    int hIter;
    static void handleTimerEvent(void* arg);
    static void handleTickerEvent(void* arg);
    static void handleTimerTimeOut(void* arg);
    static void handleTickerTimeOut(void* arg);
    static void handleTimeOut(void* arg);
    Mutex mutex;
};

struct TimeWheelEventArg {
    Time* t;
    Time* nextTime;
    TimeWheelEventArg(Time* t, Time* nextTime): t(t), nextTime(nextTime) {};
};

#endif //TIMESYSTEM_TIMEWHEEL_H
