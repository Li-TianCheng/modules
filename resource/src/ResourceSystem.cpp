//
// Created by ltc on 2021/7/21.
//

#include "resource/include/ResourceSystem.h"

void ResourceSystem::init() {
    getResourceManager();
    getThread().run(handle, nullptr);
}

void ResourceSystem::close() {
    getResourceManager()->closeCycle();
    getThread().join();
}

void ResourceSystem::registerResource(std::shared_ptr<Resource> resource, int h, int m, int s, int ms) {
    getResourceManager()->registerResource(resource, h, m, s, ms);
}

void ResourceSystem::unregisterResource(std::shared_ptr<Resource> resource) {
	getResourceManager()->unregisterResource(resource);
}

void *ResourceSystem::handle(void *) {
    getResourceManager()->cycle();
    return nullptr;
}

std::shared_ptr<ResourceManager> ResourceSystem::getResourceManager() {
    static std::shared_ptr<ResourceManager> resourceManager = ObjPool::allocate<ResourceManager>();
    return resourceManager;
}

Thread &ResourceSystem::getThread() {
    static Thread thread;
    return thread;
}

void ResourceSystem::receiveEvent(EventKey eventType, shared_ptr<void> arg) {
    auto e = ObjPool::allocate<Event>(eventType, arg);
    getResourceManager()->receiveEvent(e);
}
