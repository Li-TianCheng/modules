//
// Created by ltc on 2021/6/29.
//

#include "net/include/Buffer.h"
#include <iostream>

Buffer::Buffer() : buffer(1), readIndex(0), writeIndex(0) {
    buffer[0] = new char[BufferChunkSize];
}

iter Buffer::getReadPos() {
    return iter(&buffer, readIndex);
}

size_t Buffer::getMsgNum() {
    return writeIndex - readIndex;
}

void Buffer::write(const char* c, size_t n) {
    for (int i = 0; i < n; i++) {
        if (writeIndex == BufferChunkSize * buffer.size()) {
            buffer.push_back(new char[BufferChunkSize]);
        }
        buffer[writeIndex/BufferChunkSize][writeIndex%BufferChunkSize] = *(c+i);
        writeIndex++;
    }
}

void Buffer::readDone(size_t n) {
    readIndex += n;
    readIndex = std::min(readIndex, writeIndex);
    if (readIndex == writeIndex) {
        readIndex = 0;
        writeIndex = 0;
    }
    shrink();
}

int Buffer::readFromFd(int fd) {
    while (true) {
        if (writeIndex == BufferChunkSize * buffer.size()) {
            buffer.push_back(new char[BufferChunkSize]);
        }
        int size = BufferChunkSize - writeIndex % BufferChunkSize;
        int recvNum = ::recv(fd, buffer[writeIndex/BufferChunkSize]+writeIndex%BufferChunkSize, size, MSG_DONTWAIT);
        if (recvNum <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return 0;
            } else {
                std::cout << errno << std::endl;
                return -1;
            }
        }
        writeIndex += recvNum;
    }
}

int Buffer::writeToFd(int fd) {
    while (readIndex < writeIndex) {
        int size = std::min(BufferChunkSize-readIndex%BufferChunkSize, writeIndex-readIndex);
        int sendNum = ::send(fd, buffer[readIndex/BufferChunkSize] + readIndex % BufferChunkSize, size, MSG_DONTWAIT);
        if (sendNum <= 0) {
            if (errno == EAGAIN) {
                shrink();
                return 1;
            } else {
                shrink();
                return -1;
            }
        }
        readIndex += sendNum;
    }
    readIndex = 0;
    writeIndex = 0;
    shrink();
    return 0;
}

void Buffer::shrink() {
    while (readIndex >= BufferChunkSize) {
        buffer.push_back(buffer.front());
        buffer.pop_front();
        readIndex -= BufferChunkSize;
        writeIndex -= BufferChunkSize;
    }
    if (buffer.size() < 3) {
        return;
    }
    if ((buffer.size()-2)*BufferChunkSize > writeIndex) {
        delete[] buffer.back();
        buffer.pop_back();
    } else {
        return;
    }
    shrink();
}

Buffer::~Buffer() {
    for (auto& c : buffer) {
        delete[] c;
    }
}
