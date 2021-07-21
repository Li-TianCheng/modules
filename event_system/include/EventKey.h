//
// Created by ltc on 2021/5/16.
//

#ifndef EVENTSYSTEM_EVENTKEY_H
#define EVENTSYSTEM_EVENTKEY_H

enum EventKey{
    EventEndCycle,
    EventTimer,
    EventTicker,
    EventDeleteTicker,
    EventAddSession,
    EventTimerTimeOut,
    EventTickerTimeOut,
    EventIncrease,
    EventDecrease,
    EventRegisterResource,
    EventUnregisterResource,
    EventCloseConnection,
    EventCloseListen
};


#endif //EVENTSYSTEM_EVENTKEY_H
