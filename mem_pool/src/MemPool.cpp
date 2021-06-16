//
// Created by ltc on 2021/3/6.
//

#include "MemPool.h"

MemPool::MemPool(int num):mutex(126), num(num){
    for (int i = 0; i < 127; i++){
        mem.emplace_back(8+i*4, num);
    }
}

void* MemPool::allocate(size_t size) {
    if (size < 8 || size > 512){
        void* ptr = ::malloc(size);
        if (ptr == nullptr) {
            throw std::runtime_error("内存分配错误");
        }
        smallObjMutex.lock();
        smallObj.insert(ptr);
        smallObjMutex.unlock();
        return ptr;
    }
    mutex[(size-8)/4].lock();
    void* ptr = mem[(size-8)/4].allocate(num);
    mutex[(size-8)/4].unlock();
    return ptr;
}

void MemPool::deallocate(void *ptr, size_t size) {
    if (size < 8 || size > 512){
        smallObjMutex.lock();
        if (smallObj.find(ptr) != smallObj.end()) {
            ::free(ptr);
            smallObj.erase(ptr);
        }
        smallObjMutex.unlock();
        return;
    }
    mutex[(size-8)/4].lock();
    mem[(size-8)/4].deallocate(ptr, num);
    mutex[(size-8)/4].unlock();
}

MemPool::~MemPool() {
    bool flag = false;
    for (auto* ptr : smallObj){
        flag = true;
        ::operator delete(ptr);
    }
    if (flag) {
        std::cerr << "内存泄漏:小于8byte或大于512byte" << std::endl;
    }
}

