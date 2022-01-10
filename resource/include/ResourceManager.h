//
// Created by ltc on 2021/7/19.
//

#ifndef RESOURCE_RESOURCEMANAGER_H
#define RESOURCE_RESOURCEMANAGER_H

#include "event_system/include/EventSystem.h"
#include "Resource.h"
#include "time_system/include/TimeSystem.h"

class ResourceManager : public EventSystem {
public:
    ResourceManager();
	void registerResource(std::shared_ptr<Resource> resource, int h, int m, int s, int ms);
	void unregisterResource(std::shared_ptr<Resource> resource);
    ~ResourceManager() override = default;
private:
    void cycleClear() override;
    static void handleIncrease(shared_ptr<void> arg);
    static void handleDecrease(shared_ptr<void> arg);
    static void handleTimeOut(shared_ptr<void> arg);
    static void handleRegisterResource(shared_ptr<void> arg);
    static void handleUnregisterResource(shared_ptr<void> arg);
private:
	struct ResourceArg {
		shared_ptr<Resource> resource;
		shared_ptr<void> arg;
		ResourceArg(shared_ptr<Resource> resource, shared_ptr<void> arg) : resource(resource), arg(arg) {}
	};
	friend class ResourceSystem;
private:
    unordered_map<shared_ptr<Time>, shared_ptr<Resource>> timeToResource;
    unordered_map<shared_ptr<Resource>, shared_ptr<Time>> resourceToTime;
};




#endif //RESOURCE_RESOURCEMANAGER_H
