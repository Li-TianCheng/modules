//
// Created by ltc on 2021/5/31.
//

#include "time_system/include/Time.h"

Time::Time(int h, int m, int s, int ms, shared_ptr<EventSystem> ePtr): h(h), m(m), s(s), ms(ms), ePtr(ePtr) {

}

Time& Time::operator+=(const Time& t) {
    ms += t.ms;
    s += t.s;
    m += t.m;
    h += t.h;
    s += ms / 1000;
    m += s / 60;
    h += m / 60;
    ms %= 1000;
    s %= 60;
    m %= 60;
    h %= 24;
    return *this;
}
