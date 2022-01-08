//
// Created by ltc on 2021/3/7.
//

#include <stdexcept>
#include "Thread.h"
#include "mem_pool/include/ObjPool.hpp"

std::unordered_map<pthread_t, Thread*> Thread::ThreadMap;

Thread::Thread() : num(ConfigSystem::getConfig()["system"]["obj_pool"]["chunk_size"].asInt()) {

}

void Thread::run(void *(*handle)(void *), void* arg) {
    if (pthread_create(&threadID, nullptr, handle, arg) != 0){
        throw std::runtime_error("线程创建错误");
    }
    isRunning = true;
	ThreadMap[threadID] = this;
}

bool Thread::join() {
    if (pthread_join(threadID, &result) != 0){
        return false;
    }
    isRunning = false;
    return true;
}

bool Thread::cancel() {
    if (pthread_cancel(threadID) != 0){
        return false;
    }
    isRunning = false;
    return true;
}

pthread_t Thread::getID() const {
    return threadID;
}

bool Thread::getState() const {
    return isRunning;
}

void* Thread::getResult() const {
    return result;
}

bool Thread::detach() {
    if (pthread_detach(threadID) != 0){
        return false;
    }
    isRunning = false;
    return true;
}

void *Thread::allocate(size_t size) {
	if (size < sizeof(void*)) {
		return nullptr;
	}
	Cache* free = cache[size];
	if (free != nullptr) {
		cache[size] = free->next;
		--cacheNum[size];
	}
	return free;
}

bool Thread::deallocate(void *ptr, size_t size) {
	if (size < sizeof(void*) || cacheNum[size] >= num) {
		return false;
	}
	Cache* free = cache[size];
	((Cache*)ptr)->next = free;
	cache[size] = (Cache*)ptr;
	++cacheNum[size];
	return true;
}

void *Thread::allocateBuffer(size_t size) {
	Cache* free = buffCache[size];
	if (free != nullptr) {
		buffCache[size] = free->next;
		--cacheNum[size];
	}
	return free;
}

bool Thread::deallocateBuffer(void *ptr) {
	size_t size = *(size_t*)((char*)ptr-sizeof(size_t));
	if (cacheNum[size] >= num) {
		return false;
	}
	Cache* free = buffCache[size];
	((Cache*)ptr)->next = free;
	buffCache[size] = (Cache*)ptr;
	++cacheNum[size];
	return true;
}

Thread::~Thread() {
	ThreadMap.erase(threadID);
	for (auto& c : cache) {
		Cache* free = c.second;
		while (free != nullptr) {
			Cache* tmp = free;
			free = free->next;
			ObjPool::getInstance().deallocate(tmp, c.first);
		}
	}
	for (auto& c : buffCache) {
		Cache* free = c.second;
		while (free != nullptr) {
			Cache* tmp = free;
			free = free->next;
			ObjPool::getInstance().deallocateBuffer(tmp);
		}
	}
}
