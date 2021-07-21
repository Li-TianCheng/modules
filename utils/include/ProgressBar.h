//
// Created by ltc on 2021/5/26.
//

#ifndef UTILS_PROGRESSBAR_H
#define UTILS_PROGRESSBAR_H

#include <atomic>
#include <iostream>
#include <iomanip>
#include "resource/include/ResourceSystem.h"

using namespace std;


class ProgressBar : public Resource{
public:
    ProgressBar(const string& title, int num);
    void start();
    void done();
    void stop();
private:
    void checkOut() override;
private:
    Time curr;
    string title;
    atomic<int> count;
    int num;
};


#endif //UTILS_PROGRESSBAR_H
