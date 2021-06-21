//
// Created by ltc on 2021/6/21.
//

#include "log/include/Log.h"

Log::Log(const string &path) : isClose(false) {
    file.open(path, std::ios::out | std::ios::trunc);
    thread.run(task, this);
}

void Log::close() {
    isClose = true;
    thread.join();
    file.close();
}

void Log::log(string &&log) {
    if (isClose) {
        return;
    }
    mutex.lock();
    logQueue.push(std::forward<string>(log));
    condition.notify(mutex);
}

void *Log::task(void *arg) {
    Log* log = (Log*)arg;
    while(!log->isClose || !log->logQueue.empty()) {
        log->mutex.lock();
        while(log->logQueue.empty()) {
            log->condition.wait(log->mutex);
        }
        vector<string> temp;
        while (!log->logQueue.empty()) {
            temp.push_back(move(log->logQueue.front()));
            log->logQueue.pop();
        }
        log->condition.notify(log->mutex);
        for (auto& s : temp) {
            log->file << s << std::endl;
        }
    }
}
