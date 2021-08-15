//
// Created by ltc on 2021/6/29.
//

#include "net/include/Buffer.h"


Buffer::Buffer(int bufferChunkSize) : bufferChunkSize(bufferChunkSize), buffer(1), readIndex(0), writeIndex(0) {
    buffer[0] = new char[bufferChunkSize];
}

iter Buffer::getReadPos() {
    return iter(bufferChunkSize, &buffer, readIndex);
}

size_t Buffer::getMsgNum() {
    return writeIndex - readIndex;
}

void Buffer::write(const char* c, size_t n) {
    for (int i = 0; i < n; i++) {
        if (writeIndex == bufferChunkSize * buffer.size()) {
            buffer.push_back(new char[bufferChunkSize]);
        }
        buffer[writeIndex/bufferChunkSize][writeIndex%bufferChunkSize] = *(c+i);
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
        if (writeIndex == bufferChunkSize * buffer.size()) {
            buffer.push_back(new char[bufferChunkSize]);
        }
        size_t size = bufferChunkSize - writeIndex % bufferChunkSize;
        ssize_t recvNum = ::recv(fd, buffer[writeIndex/bufferChunkSize]+writeIndex%bufferChunkSize, size, MSG_DONTWAIT);
        if (recvNum <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return 0;
            } else {
                return -1;
            }
        }
        writeIndex += recvNum;
    }
}

int Buffer::writeToFd(int fd) {
    while (readIndex < writeIndex) {
        size_t size = std::min(bufferChunkSize-readIndex%bufferChunkSize, writeIndex-readIndex);
        ssize_t sendNum = ::send(fd, buffer[readIndex/bufferChunkSize] + readIndex % bufferChunkSize, size, MSG_DONTWAIT);
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
    while (readIndex >= bufferChunkSize) {
        buffer.push_back(buffer.front());
        buffer.pop_front();
        readIndex -= bufferChunkSize;
        writeIndex -= bufferChunkSize;
    }
    if (buffer.size() < 3) {
        return;
    }
    if ((buffer.size()-2)*bufferChunkSize > writeIndex) {
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

void Buffer::copy(const iter &begin, size_t n, string& buff) {
    size_t total = 0;
    size_t curr = begin.readIndex;
    while (total < n) {
        size_t remaining = bufferChunkSize - curr % bufferChunkSize;
        size_t inc = remaining < n-total ? remaining : n-total;
        buff.insert(buff.end(), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize)+inc);
        total += inc;
        curr += inc;
    }
}

void Buffer::copy(const iter &begin, size_t n, vector<char>& buff) {
    size_t total = 0;
    size_t curr = begin.readIndex;
    while (total < n) {
        size_t remaining = bufferChunkSize - curr % bufferChunkSize;
        size_t inc = remaining < n-total ? remaining : n-total;
        buff.insert(buff.end(), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize)+inc);
        total += inc;
        curr += inc;
    }
}

void Buffer::copy(const iter &begin, size_t n, vector<unsigned char>& buff) {
    size_t total = 0;
    size_t curr = begin.readIndex;
    while (total < n) {
        size_t remaining = bufferChunkSize - curr % bufferChunkSize;
        size_t inc = remaining < n-total ? remaining : n-total;
        buff.insert(buff.end(), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize), buffer[curr/bufferChunkSize]+(curr%bufferChunkSize)+inc);
        total += inc;
        curr += inc;
    }
}
