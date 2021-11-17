//
// Created by ltc on 2021/6/29.
//

#ifndef NET_BUFFER_H
#define NET_BUFFER_H

#include <deque>
#include <cerrno>
#include <sys/socket.h>
#include <string>
#include <memory>
#include <vector>
#include "mem_pool/include/ObjPool.hpp"

using std::deque;
using std::string;
using std::vector;
using std::shared_ptr;

struct iter;

class Buffer {
public:
    explicit Buffer(int bufferChunkSize);
    iter getReadPos();
    size_t getMsgNum();
    void write(const char* c, size_t n);
    void copy(const iter& begin, size_t n, string& buff);
    void copy(const iter& begin, size_t n, vector<char>& buff);
    void copy(const iter& begin, size_t n, vector<unsigned char>& buff);
    void readDone(size_t n);
    int readFromFd(int fd);
    int writeToFd(int fd);
    ~Buffer();
private:
    void shrink();
private:
    deque<char*> buffer;
    size_t readIndex;
    size_t writeIndex;
    int bufferChunkSize;
};

struct iter {
public:
    iter(int bufferChunkSize, deque<char*>* buffer, size_t readIndex) : bufferChunkSize(bufferChunkSize), buffer(buffer), readIndex(readIndex){}
    iter& operator++(){
        readIndex++;
        return *this;
    }
    iter operator++(int){
        iter it(bufferChunkSize, buffer, readIndex);
        readIndex++;
        return it;
    }
    iter& operator--(){
        readIndex--;
        return *this;
    }
    iter operator--(int){
        iter it(bufferChunkSize, buffer, readIndex);
        readIndex--;
        return it;
    }
    bool operator==(const iter& iter) const {
        return readIndex == iter.readIndex;
    }
    bool operator!=(const iter& iter) const {
        return readIndex != iter.readIndex;
    }
    iter& operator+=(int n) {
        readIndex += n;
        return *this;
    }
    iter& operator-=(int n) {
        readIndex -= n;
        return *this;
    }
    iter operator+(int n){
        iter it(bufferChunkSize, buffer, readIndex);
        it.readIndex += n;
        return it;
    };
    iter operator-(int n){
        iter it(bufferChunkSize, buffer, readIndex);
        it.readIndex -= n;
        return it;
    };
    char& operator*() const{
        return (*buffer)[readIndex/bufferChunkSize][readIndex % bufferChunkSize];
    }
private:
    friend class Buffer;
    deque<char*>* buffer;
    size_t readIndex;
    int bufferChunkSize;
};


#endif //NET_BUFFER_H
