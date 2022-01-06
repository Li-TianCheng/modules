//
// Created by ltc on 2021/7/19.
//

#include "resource/include/ResourceManager.h"

ResourceManager::ResourceManager() {
    registerEvent(EventIncrease, handleIncrease);
    registerEvent(EventDecrease, handleDecrease);
    registerEvent(EventTickerTimeOut, handleTimeOut);
    registerEvent(EventRegisterResource, handleRegisterResource);
    registerEvent(EventUnregisterResource, handleUnregisterResource);
    registerEvent(EventEndCycle, nullptr);
}

void ResourceManager::handleIncrease(shared_ptr<void> arg) {
    static_pointer_cast<Resource>(arg)->increase();
}

void ResourceManager::handleDecrease(shared_ptr<void> arg) {
    static_pointer_cast<Resource>(arg)->decrease();
}

void ResourceManager::handleTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<Time>(arg);
    auto rm = static_pointer_cast<ResourceManager>(t->ePtr.lock());
    if (rm != nullptr && rm->timeToResource.find(t) != rm->timeToResource.end()) {
        rm->timeToResource[t]->checkOut();
    }
}

void ResourceManager::handleRegisterResource(shared_ptr<void> arg) {
    auto r = static_pointer_cast<ResourceArg>(arg);
    auto t = static_pointer_cast<Time>(r->arg);
    auto rm = static_pointer_cast<ResourceManager>(t->ePtr.lock());
    if (rm != nullptr) {
        rm->timeToResource[t] = r->resource;
        rm->resourceToTime[r->resource] = t;
    }
    TimeSystem::receiveEvent(EventTicker, t);
}

void ResourceManager::handleUnregisterResource(shared_ptr<void> arg) {
    auto r = static_pointer_cast<ResourceArg>(arg);
    auto rm = static_pointer_cast<ResourceManager>(r->arg);
    if (rm != nullptr) {
        if (rm->resourceToTime.find(r->resource) != rm->resourceToTime.end()) {
            rm->timeToResource.erase(rm->resourceToTime[r->resource]);
            rm->resourceToTime.erase(r->resource);
        }
    }
}

void ResourceManager::cycleClear() {
    EventSystem::cycleClear();
    timeToResource.clear();
    resourceToTime.clear();
}
