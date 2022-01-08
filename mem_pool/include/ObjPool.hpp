//
// Created by ltc on 2021/5/8.
//

#ifndef MEMPOOL_OBJPOOL_HPP
#define MEMPOOL_OBJPOOL_HPP

#include <memory>
#include "MemPool.h"
#include "config_system/include/ConfigSystem.h"
#include "my_pthread/include/Thread.h"

using std::shared_ptr;
using std::weak_ptr;
using std::static_pointer_cast;

class ObjPool{
public:
    template<typename T, typename... Args>
    static shared_ptr<T> allocate(Args&&... args);
    static void init();
	template<typename T>
	static shared_ptr<T> allocateBuffer(size_t size);
    ObjPool() = delete;
    ObjPool(const ObjPool&) = delete;
    ObjPool(ObjPool&&) = delete;
    ObjPool& operator=(const ObjPool&) = delete;
    ObjPool& operator=(ObjPool&&) = delete;
private:
	friend class Thread;
private:
    template<typename T>
    static void deallocate(T* ptr);
	static void deallocateBuffer(void* ptr);
    static MemPool& getInstance();
};

template<typename T, typename... Args> inline
shared_ptr<T> ObjPool::allocate(Args&&... args) {
	pthread_t pid = pthread_self();
	void* ptr = nullptr;
	if (Thread::ThreadMap.find(pid) != Thread::ThreadMap.end()) {
		ptr = Thread::ThreadMap[pid]->allocate(sizeof(T));
	}
	if (ptr == nullptr) {
		ptr = getInstance().allocate(sizeof(T));
	}
    return shared_ptr<T>(::new(ptr) T(std::forward<Args>(args)...), deallocate<T>);
}

template<typename T> inline
void ObjPool::deallocate(T* ptr) {
    ptr->~T();
	pthread_t pid = pthread_self();
	if (Thread::ThreadMap.find(pid) != Thread::ThreadMap.end()) {
		if (!Thread::ThreadMap[pid]->deallocate(ptr, sizeof(T))) {
			getInstance().deallocate(ptr, sizeof(T));
		}
	} else {
		getInstance().deallocate(ptr, sizeof(T));
	}
}
template<typename T> inline
shared_ptr<T> ObjPool::allocateBuffer(size_t size) {
	pthread_t pid = pthread_self();
	void* ptr = nullptr;
	if (Thread::ThreadMap.find(pid) != Thread::ThreadMap.end()) {
		ptr = Thread::ThreadMap[pid]->allocateBuffer(size*sizeof(T));
	}
	if (ptr == nullptr) {
		ptr = getInstance().allocateBuffer(size*sizeof(T));
	}
	return shared_ptr<T>((T*)ptr, deallocateBuffer);
}

inline void ObjPool::deallocateBuffer(void* ptr) {
	pthread_t pid = pthread_self();
	if (Thread::ThreadMap.find(pid) != Thread::ThreadMap.end()) {
		if (!Thread::ThreadMap[pid]->deallocateBuffer(ptr)) {
			getInstance().deallocateBuffer(ptr);
		}
	} else {
		getInstance().deallocateBuffer(ptr);
	}
}

inline MemPool& ObjPool::getInstance() {
    static MemPool memPool(ConfigSystem::getConfig()["system"]["obj_pool"]["chunk_size"].asInt());
    return memPool;
}

inline void ObjPool::init() {
    getInstance();
}

#endif //MEMPOOL_OBJPOOL_HPP
