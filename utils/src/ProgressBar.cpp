//
// Created by ltc on 2021/5/26.
//

#include "ProgressBar.h"

ProgressBar::ProgressBar(const string &title, int num) :curr(0,0,0,0,nullptr), updateTime(nullptr),
    title(title), num(num), count(0) {
}

void ProgressBar::start() {
    ResourceSystem::registerResource(shared_from_this(), 0, 0, 0, 500);
    for (int i = 0; i < 60-title.size(); i++){
        cout << " ";
    }
    cout << title << endl;
    checkOut();
}

void ProgressBar::done() {
    if (count >= num) {
        checkOut();
    } else {
        count++;
    }
}

void ProgressBar::stop() {
    cout << endl;
    ResourceSystem::unregisterResource(shared_from_this());
}

void ProgressBar::checkOut() {
    curr += *updateTime;
    cout << "\r";
    cout << "[" << count << "/" << num;
    cout << "|" << fixed << setprecision(2) << double(count)/num*100 << "%]";
    int percent = count * 100 / num;
    for (int i = 0; i < percent; i++) {
        cout << "■";
    }
    for (int i = percent; i < 100; i++) {
        cout << " ";
    }
    int total = curr.ms + 1000*curr.s + 1000*60*curr.m + 1000*24*60*curr.h;
    double speed = double(count) / total;
    int remaining = (int)((num-count) / speed);
    int remainingH = remaining / 3600000;
    remaining %= 3600000;
    int remainingM = remaining / 60000;
    remaining %= 60000;
    int remainingS = remaining / 1000;
    cout << "[" << fixed << setprecision(2) << speed*1000 << "/s";
    cout << "|" << setw(2) << setfill('0') << curr.h << ":";
    cout << setw(2) << setfill('0') << curr.m << ":";
    cout << setw(2) << setfill('0') << curr.s;
    cout << "|" << setw(2) << setfill('0') << remainingH << ":";
    cout << setw(2) << setfill('0') << remainingM << ":";
    cout << setw(2) << setfill('0') << remainingS << "]";
    cout << flush;
    if (count == num) {
        stop();
    }
}
