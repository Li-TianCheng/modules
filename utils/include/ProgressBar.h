//
// Created by ltc on 2021/5/26.
//

#ifndef UTILS_PROGRESSBAR_H
#define UTILS_PROGRESSBAR_H

#include <atomic>
#include <iostream>
#include <iomanip>
#include "event_system/include/EventSystem.h"
#include "task_system/include/TaskSystem.h"
#include "time_system/include/TimeSystem.h"

using namespace std;


class ProgressBar : public EventSystem{
public:
    ProgressBar(const string& title, int num);
    void start();
    void done();
    void stop();
    void cycleInit() override;
    void cycleClear() override;
private:
    void init();
    static void handleTimeOut(void* arg);
    void draw();
private:
    Time updateTime;
    Time curr;
    string title;
    atomic<int> count;
    int num;
    string uuid;
};


#endif //UTILS_PROGRESSBAR_H
