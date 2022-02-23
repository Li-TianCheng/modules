//
// Created by ltc on 2021/6/29.
//

#include "net/include/Buffer.h"
#include "Session.h"

Buffer::Buffer(int bufferChunkSize) : bufferChunkSize(bufferChunkSize), buffer(1), readIndex(0), writeIndex(0) {
	buffer[0] = ObjPool::allocateBuffer<unsigned char>(bufferChunkSize);
}

iter Buffer::getReadPos() {
    return iter(bufferChunkSize, &buffer, readIndex);
}

size_t Buffer::getMsgNum() {
    return writeIndex - readIndex;
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
            buffer.push_back(ObjPool::allocateBuffer<unsigned char>(bufferChunkSize));
        }
        size_t size = bufferChunkSize - writeIndex % bufferChunkSize;
        ssize_t recvNum = ::recv(fd, buffer[writeIndex/bufferChunkSize].get()+writeIndex%bufferChunkSize, size, MSG_DONTWAIT);
        if (recvNum <= 0) {
            if (errno == EAGAIN) {
                return 0;
            } else {
                return -1;
            }
        }
        writeIndex += recvNum;
    }
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
        buffer.pop_back();
    } else {
        return;
    }
    shrink();
}

void Buffer::copy(const iter &begin, size_t n, string& buff) {
    size_t total = 0;
    size_t curr = begin.readIndex;
    while (total < n) {
        size_t remaining = bufferChunkSize - curr % bufferChunkSize;
        size_t inc = remaining < n-total ? remaining : n-total;
        buff.insert(buff.end(), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize)+inc);
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
        buff.insert(buff.end(), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize)+inc);
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
        buff.insert(buff.end(), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize), buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize)+inc);
        total += inc;
        curr += inc;
    }
}

void Buffer::copy(const iter& begin, size_t n, char* buff) {
	size_t total = 0;
	size_t curr = begin.readIndex;
	while (total < n) {
		size_t remaining = bufferChunkSize - curr % bufferChunkSize;
		size_t inc = remaining < n-total ? remaining : n-total;
		memcpy(buff, buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize), inc);
		total += inc;
		curr += inc;
	}
}

void Buffer::copy(const iter& begin, size_t n, unsigned char* buff) {
	size_t total = 0;
	size_t curr = begin.readIndex;
	while (total < n) {
		size_t remaining = bufferChunkSize - curr % bufferChunkSize;
		size_t inc = remaining < n-total ? remaining : n-total;
		memcpy(buff, buffer[curr/bufferChunkSize].get()+(curr%bufferChunkSize), inc);
		total += inc;
		curr += inc;
	}
}

void Buffer::copy(const iter& begin, size_t n, Buffer& buff) {
	buff.insert(*this, begin.readIndex, begin.readIndex+n);
}

void Buffer::insert(const unsigned char *c, size_t len) {
	size_t curr = 0;
	while (true) {
		if (writeIndex == buffer.size()*bufferChunkSize) {
			buffer.push_back(ObjPool::allocateBuffer<unsigned char>(bufferChunkSize));
		}
		size_t remain = bufferChunkSize-writeIndex%bufferChunkSize;
		if (remain >= len) {
			memcpy(buffer[writeIndex/bufferChunkSize].get()+writeIndex%bufferChunkSize, c+curr, len);
			writeIndex += len;
			break;
		} else {
			memcpy(buffer[writeIndex/bufferChunkSize].get()+writeIndex%bufferChunkSize, c+curr, remain);
			curr += remain;
			len -= remain;
			writeIndex += remain;
		}
	}
}

void Buffer::insert(Buffer &buff, size_t begin, size_t end) {
	size_t curr = begin;
	while (true) {
		if (curr / buff.bufferChunkSize == end / buff.bufferChunkSize) {
			insert(buff.buffer[curr/buff.bufferChunkSize].get()+curr%buff.bufferChunkSize, end-curr);
			break;
		} else {
			insert(buff.buffer[curr/buff.bufferChunkSize].get()+curr%buff.bufferChunkSize, (curr/buff.bufferChunkSize+1)*buff.bufferChunkSize-curr);
			curr = (curr/buff.bufferChunkSize+1)*buff.bufferChunkSize;
		}
	}
}

void Buffer::push_back(unsigned char c) {
	if (writeIndex == buffer.size()*bufferChunkSize) {
		buffer.push_back(ObjPool::allocateBuffer<unsigned char>(bufferChunkSize));
	}
	buffer[writeIndex/bufferChunkSize].get()[writeIndex%bufferChunkSize] = c;
	++writeIndex;
}

unsigned char &Buffer::operator[](size_t index) {
	return buffer[index/bufferChunkSize].get()[index%bufferChunkSize];
}

void Buffer::swap(Buffer &buff) {
	std::swap(bufferChunkSize, buff.bufferChunkSize);
	std::swap(writeIndex, buff.writeIndex);
	buffer.swap(buff.buffer);
}

void Buffer::clear() {
	writeIndex = 0;
}

size_t Buffer::size() const {
	return writeIndex;
}
