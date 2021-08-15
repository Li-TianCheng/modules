//
// Created by ltc on 2021/7/21.
//

#ifndef RESOURCE_RESOURCESYSTEM_H
#define RESOURCE_RESOURCESYSTEM_H

#include "Resource.h"
#include "ResourceManager.h"
#include "my_pthread/include/Thread.h"
#include "mem_pool/include/ObjPool.hpp"

class ResourceSystem {
public:
    static void init();
    static void close();
    static void registerResource(std::shared_ptr<Resource> resource);
    static void registerResource(std::shared_ptr<Resource> resource, int h, int m, int s, int ms);
    static void unregisterResource(std::shared_ptr<Resource> resource);
    static void receiveEvent(EventKey eventType, shared_ptr<void> arg);
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem(ResourceSystem&&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;
    ResourceSystem& operator=(ResourceSystem&&) = delete;
private:
    static void* handle(void*);
    ResourceSystem() = default;
    static std::shared_ptr<ResourceManager> getResourceManager();
    static Thread& getThread();
};


#endif //RESOURCE_RESOURCESYSTEM_H
