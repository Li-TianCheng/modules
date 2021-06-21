//
// Created by ltc on 2021/6/21.
//

#ifndef LOG_LOG_H
#define LOG_LOG_H

#include <string>
#include <iostream>
#include <fstream>
#include <queue>
#include <sys/time.h>
#include <vector>
#include "my_pthread/include/Condition.h"
#include "my_pthread/include/Thread.h"

using std::string;
using std::queue;
using std::vector;

enum LogRank{
    Info,
    Warn,
    Error,
    Debug,
    Trace
};

static const string logString[]{
    "Info",
    "Warn",
    "Error",
    "Debug",
    "Trace"
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

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define LOG(logger, rank, msg) logger.log(logString[rank]+" "+getTime()+" "+__FILE_NAME__+":"+to_string(__LINE__)+" "+std::move(msg))

class Log {
public:
    explicit Log(const string& path);
    void close();
    void log(string&& log);
private:
    static void* task(void* arg);
private:
    std::ofstream file;
    volatile bool isClose;
    Mutex mutex;
    Condition condition;
    queue<string> logQueue;
    Thread thread;
};


#endif //LOG_LOG_H
