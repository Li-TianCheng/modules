//
// Created by ltc on 2021/3/6.
//

#include "MemPool.h"

MemPool::MemPool(int num):mutex(126), num(num){
    for (int i = 0; i < 128; i++){
        mem.emplace_back(4+i*4, num);
    }
}

void* MemPool::allocate(size_t size) {
    if (size < 4 || size > 512){
        void* ptr = ::malloc(size);
        if (ptr == nullptr) {
            throw std::runtime_error("内存分配错误");
        }
        return ptr;
    }
    mutex[(size-4)/4].lock();
    void* ptr = mem[(size-4)/4].allocate(num);
    mutex[(size-4)/4].unlock();
    return ptr;
}

void MemPool::deallocate(void *ptr, size_t size) {
    if (size < 4 || size > 512){
        ::free(ptr);
        return;
    }
    mutex[(size-4)/4].lock();
    mem[(size-4)/4].deallocate(ptr, num);
    mutex[(size-4)/4].unlock();
}

