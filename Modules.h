//
// Created by ltc on 2021/5/17.
//

#ifndef MODULES_MODULES_H
#define MODULES_MODULES_H

#include "mem_pool/include/ObjPool.hpp"
#include "time_system/include/TimeSystem.h"
#include "resource/include/ResourceSystem.h"
#include "task_system/include/TaskSystem.h"

namespace modules {
    static void init();
    static void close();
}

inline void modules::init(){
    ObjPool::init();
    TimeSystem::init();
    ResourceSystem::init();
    TaskSystem::init();
}

inline void modules::close(){
    TaskSystem::close();
    ResourceSystem::close();
    TimeSystem::close();
    ObjPool::close();
}

#endif //MODULES_MODULES_H
