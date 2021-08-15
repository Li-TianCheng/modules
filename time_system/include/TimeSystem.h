//
// Created by ltc on 2021/5/14.
//

#ifndef TIMESYSTEM_TIMESYSTEM_H
#define TIMESYSTEM_TIMESYSTEM_H

#include <sys/epoll.h>
#include "TimeWheel.h"
#include "my_pthread/include/Thread.h"

class TimeSystem{
public:
    static void init();
    static void close();
    static void receiveEvent(EventKey eventType, shared_ptr<Time> arg);
    static void deleteTicker(shared_ptr<Time> arg);
    TimeSystem(const TimeSystem&) = delete;
    TimeSystem(TimeSystem&&) = delete;
    TimeSystem& operator=(const TimeSystem&) = delete;
    TimeSystem& operator=(TimeSystem&&) = delete;
private:
    static void* handle(void*);
    TimeSystem() = default;
    static shared_ptr<TimeWheel> getTimeWheel();
    static Thread& getThread();
};




#endif //TIMESYSTEM_TIMESYSTEM_H
