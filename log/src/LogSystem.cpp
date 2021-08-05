//
// Created by ltc on 2021/7/21.
//

#include "log/include/LogSystem.h"

string LogSystem::path;
int LogSystem::rank;

void LogSystem::init(const string& path) {
    LogSystem::path = path;
    LogSystem::rank = ConfigSystem::getConfig()["system"]["log"]["rank"].asInt();
    getLog();
    auto time = ConfigSystem::getConfig()["system"]["log"]["check_time"];
    ResourceSystem::registerResource(getLog(), time[0].asInt(), time[1].asInt(), time[2].asInt(), time[3].asInt());
}

void LogSystem::close() {
    while (!getLog()->logQueue.empty()){
        sleep(1);
    }
    ResourceSystem::unregisterResource(getLog());
    getLog()->file.close();
}

void LogSystem::log(LogRank rank, string &&str) {
    if (rank >= LogSystem::rank) {
        getLog()->log(logString[rank]+" "+getTime()+" "+std::forward<string>(str));
    }
}

shared_ptr<Log> LogSystem::getLog() {
    static shared_ptr<Log> log = ObjPool::allocate<Log>(LogSystem::path);
    return log;
}
