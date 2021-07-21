//
// Created by ltc on 2021/5/17.
//

#ifndef MODULES_MODULES_H
#define MODULES_MODULES_H

#include "mem_pool/include/ObjPool.hpp"
#include "time_system/include/TimeSystem.h"
#include "resource/include/ResourceSystem.h"
#include "task_system/include/TaskSystem.h"
#include "log/include/LogSystem.h"

namespace modules {
    static void init(const string& path="../log/log.log");
    static void close();
}

inline void modules::init(const string& path){
    ObjPool::init();
    TimeSystem::init();
    ResourceSystem::init();
    LogSystem::init(path);
    TaskSystem::init();
}

inline void modules::close(){
    TaskSystem::close();
    LogSystem::close();
    ResourceSystem::close();
    TimeSystem::close();
    ObjPool::close();
}

#endif //MODULES_MODULES_H
