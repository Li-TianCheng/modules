//
// Created by ltc on 2021/7/21.
//

#ifndef LOG_LOGSYSTEM_H
#define LOG_LOGSYSTEM_H

#include "Log.h"

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define LOG(rank, msg) LogSystem::log(logString[rank]+" "+getTime()+" "+__FILE_NAME__+":"+std::to_string(__LINE__)+" "+std::move(msg))

class LogSystem {
public:
    static void init(const string& path);
    static void close();
    static void log(string&& str);
    LogSystem(const LogSystem&) = delete;
    LogSystem(LogSystem&&) = delete;
    LogSystem& operator=(const LogSystem&) = delete;
    LogSystem& operator=(LogSystem&&) = delete;
private:
    LogSystem() = default;
    static shared_ptr<Log> getLog();
    static string path;
};


#endif //LOG_LOGSYSTEM_H
