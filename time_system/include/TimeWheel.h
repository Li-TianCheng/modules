//
// Created by ltc on 2021/5/14.
//

#ifndef TIMESYSTEM_TIMEWHEEL_H
#define TIMESYSTEM_TIMEWHEEL_H

#include <vector>
#include <unordered_set>
#include <unistd.h>
#include <sys/epoll.h>
#include <queue>
#include "event_system/include/EventSystem.h"
#include "Time.h"

using std::vector;
using std::queue;
using std::unordered_set;

class TimeWheel: public EventSystem {
public:
    TimeWheel();
    ~TimeWheel() override;
    void timeWheelCycle();
    TimeWheel(const TimeWheel&) = delete;
    TimeWheel(TimeWheel&&) = delete;
    TimeWheel& operator=(const TimeWheel&) = delete;
    TimeWheel& operator=(TimeWheel&&) = delete;
private:
    void init();
    void addTimeToWheel(EventKey e, shared_ptr<Time> t);
    void update();
    static void handleDeleteTicker(shared_ptr<void> arg);
    static void handleTimerEvent(shared_ptr<void> arg);
    static void handleTickerEvent(shared_ptr<void> arg);
    static void handleTimerTimeOut(shared_ptr<void> arg);
    static void handleTickerTimeOut(shared_ptr<void> arg);
    static void handleEndCycle(shared_ptr<void> arg);
private:
    vector<queue<shared_ptr<Event>>> millisecond;
    vector<queue<shared_ptr<Event>>> second;
    vector<queue<shared_ptr<Event>>> minute;
    vector<queue<shared_ptr<Event>>> hour;
    std::atomic<bool> shutdown;
    int epollFd;
    int msIter;
    int sIter;
    int mIter;
    int hIter;
};

struct TimeWheelEventArg {
    shared_ptr<Time> t;
    shared_ptr<Time> nextTime;
    TimeWheelEventArg(shared_ptr<Time> t, shared_ptr<Time> nextTime): t(t), nextTime(nextTime) {};
};

#endif //TIMESYSTEM_TIMEWHEEL_H
