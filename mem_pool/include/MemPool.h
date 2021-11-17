//
// Created by ltc on 2021/3/6.
//

#ifndef MEMPOOL_MEMPOOL_H
#define MEMPOOL_MEMPOOL_H

#include "ManageChunk.h"
#include "../../my_pthread/include/Mutex.h"
#include <vector>
#include <unordered_map>
#include <iostream>

using std::vector;
using std::unordered_map;

struct bufferChunk {
	bufferChunk* next;
};

class MemPool {
public:
    explicit MemPool(int num);
    void* allocate(size_t size);
    void deallocate(void* ptr, size_t size);
	void* allocateBuffer(size_t size);
	void deallocateBuffer(void* ptr, size_t size);
    ~MemPool();
    MemPool(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool& operator=(MemPool&&) = delete;
private:
    vector<ManageChunk> mem;
    vector<Mutex> mutex;
	Mutex bufferMutex;
	unordered_map<size_t, bufferChunk*> bufferMem;
    int num;
};


#endif //MEMPOOL_MEMPOOL_H
