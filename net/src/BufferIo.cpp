//
// Created by ltc on 2021/5/24.
//

#include "BufferIo.h"

string BufferIo::read(int socketFd) {
    int recvNum = recv(socketFd, readBuffer, sizeof(readBuffer), 0);
    string context = readBuffer;
    context.resize(recvNum);
    return context;
}

void BufferIo::write(int socketFd, const string& context) {
    int idx = 0;
    while (idx < context.length()) {
        send(socketFd, context.substr(idx, writeBufferSize).data(), context.size(), 0);
        idx += writeBufferSize;
    }
}