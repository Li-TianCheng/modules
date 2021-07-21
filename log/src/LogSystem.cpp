//
// Created by ltc on 2021/7/21.
//

#include "log/include/LogSystem.h"

string LogSystem::path;

void LogSystem::init(const string& path) {
    LogSystem::path = path;
    getLog();
    ResourceSystem::registerResource(getLog(), 0, 0, 0, 10);
}

void LogSystem::close() {
    while (!getLog()->logQueue.empty()){
        sleep(1);
    }
    ResourceSystem::unregisterResource(getLog());
    getLog()->file.close();
}

void LogSystem::log(string &&str) {
    getLog()->log(std::forward<string>(str));
}

shared_ptr<Log> LogSystem::getLog() {
    static shared_ptr<Log> log = ObjPool::allocate<Log>(LogSystem::path);
    return log;
}
