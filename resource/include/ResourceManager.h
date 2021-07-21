//
// Created by ltc on 2021/7/19.
//

#ifndef RESOURCE_RESOURCEMANAGER_H
#define RESOURCE_RESOURCEMANAGER_H

#include "event_system/include/EventSystem.h"
#include "Resource.h"
#include "time_system/include/TimeSystem.h"

struct CheckTime : public Time {
    shared_ptr<Resource> r;
};

class ResourceManager : public EventSystem {
public:
    ResourceManager();
    ~ResourceManager() override = default;
private:
    static void handleIncrease(shared_ptr<void> arg);
    static void handleDecrease(shared_ptr<void> arg);
    static void handleTimeOut(shared_ptr<void> arg);
    static void handleRegisterResource(shared_ptr<void> arg);
    static void handleUnregisterResource(shared_ptr<void> arg);
private:
    unordered_map<shared_ptr<Time>, shared_ptr<Resource>> timeToResource;
    unordered_map<shared_ptr<Resource>, shared_ptr<Time>> resourceToTime;
};

struct ResourceArg {
    shared_ptr<Resource> resource;
    shared_ptr<void> arg;
    ResourceArg(shared_ptr<Resource> resource, shared_ptr<void> arg) : resource(resource), arg(arg) {}
};


#endif //RESOURCE_RESOURCEMANAGER_H
