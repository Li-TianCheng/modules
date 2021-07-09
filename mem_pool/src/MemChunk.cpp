//
// Created by ltc on 2021/3/6.
//

#include "MemChunk.h"
#include <malloc.h>
#include <new>

MemChunk::MemChunk(int num, size_t size):num(num), free(nullptr), start(nullptr), prev(nullptr), next(nullptr), size(size){
    while (true){
        free = (obj*)malloc(num * size);
        if (free != nullptr){
            break;
        }else{
            throw std::bad_alloc();
        }
    }
    start = free;
    for (int i = 0; i < num-1; i++){
        free->next = i+1;
        free = (obj*)((char*)free+size);
    }
    free = start;
}

void* MemChunk::allocate() {
    if (num == 0) {
        return nullptr;
    }
    obj* curr = free;
    free = (obj*)((char*)start+(free->next)*size);
    num--;
    return curr;
}

void MemChunk::deallocate(void *ptr) {
    ((obj*)ptr)->next = ((char*)free-(char*)start) / size;
    free = (obj*)ptr;
    num++;
}

MemChunk::~MemChunk() {
    ::free(start);
}