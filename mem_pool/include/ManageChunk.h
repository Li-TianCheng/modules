//
// Created by ltc on 2021/3/6.
//

#ifndef MEMPOOL_MANAGECHUNK_H
#define MEMPOOL_MANAGECHUNK_H

#include <iostream>
#include "MemChunk.h"

class ManageChunk {
public:
    explicit ManageChunk(size_t size, int num);
    void* allocate(int num);
    void deallocate(void* ptr, int num);
    ~ManageChunk();
private:
    MemChunk* head;
    MemChunk* free;
    size_t size;
    int num;
};


#endif //MEMPOOL_MANAGECHUNK_H
