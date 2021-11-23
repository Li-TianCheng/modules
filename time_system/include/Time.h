//
// Created by ltc on 2021/5/31.
//

#ifndef TIMESYSTEM_TIME_H
#define TIMESYSTEM_TIME_H

#include "event_system/include/EventSystem.h"

class TimeWheel;

class Time {
public:
    Time(int h, int m, int s, int ms, shared_ptr<EventSystem> ePtr);
    Time& operator+=(const Time& t);
public:
	int ms;
	int s;
	int m;
	int h;
	weak_ptr<EventSystem> ePtr;
	weak_ptr<TimeWheel> tPtr;
};


#endif //TIMESYSTEM_TIME_H
