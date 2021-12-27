//
// Created by ltc on 2021/5/8.
//

#ifndef MEMPOOL_OBJPOOL_HPP
#define MEMPOOL_OBJPOOL_HPP

#include <memory>
#include "MemPool.h"
#include "config_system/include/ConfigSystem.h"

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
    template<typename T>
    static void deallocate(T* ptr);
	static void deallocateBuffer(void* ptr);
    static MemPool& getInstance();
};

template<typename T, typename... Args> inline
shared_ptr<T> ObjPool::allocate(Args&&... args) {
    void* ptr = getInstance().allocate(sizeof(T));
    return shared_ptr<T>(::new(ptr) T(std::forward<Args>(args)...), deallocate<T>);
}

template<typename T> inline
void ObjPool::deallocate(T* ptr) {
    ptr->~T();
    getInstance().deallocate(ptr, sizeof(T));
}
template<typename T> inline
shared_ptr<T> ObjPool::allocateBuffer(size_t size) {
	void* ptr = getInstance().allocateBuffer(size*sizeof(T));
	return shared_ptr<T>((T*)ptr, deallocateBuffer);
}

inline void ObjPool::deallocateBuffer(void* ptr) {
	getInstance().deallocateBuffer(ptr);
}

inline MemPool& ObjPool::getInstance() {
    static MemPool memPool(ConfigSystem::getConfig()["system"]["obj_pool"]["chunk_size"].asInt());
    return memPool;
}

inline void ObjPool::init() {
    getInstance();
}

#endif //MEMPOOL_OBJPOOL_HPP
