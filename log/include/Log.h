//
// Created by ltc on 2021/6/21.
//

#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <cstring>
#include <sys/time.h>
#include <vector>
#include <atomic>
#include "resource/include/ResourceSystem.h"

using std::string;
using std::queue;
using std::vector;

enum LogRank{
    Trace,
    Debug,
    Info,
    Access,
    Warn,
    Error,
};

static const string logString[]{
    "Trace",
    "Debug",
    "Info",
    "Access",
    "Warn",
    "Error"
};

static string getTime() {
    timeval curTime;
    gettimeofday(&curTime, nullptr);
    char buffer[80] = {0};
    tm nowTime;
    localtime_r(&curTime.tv_sec, &nowTime);//把得到的值存入临时分配的内存中，线程安全
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &nowTime);
    char currentTime[84] = {0};
    snprintf(currentTime, sizeof(currentTime), "%s:%ld", buffer, curTime.tv_usec);
    return currentTime;
}

class Log : public Resource {
public:
    explicit Log(const string& path);
    void log(string&& log);
	~Log() override = default;
private:
    void checkOut() override;
    friend class LogSystem;
private:
    std::ofstream file;
    Mutex mutex;
    queue<string> logQueue;
};


#endif //LOG_LOG_H
