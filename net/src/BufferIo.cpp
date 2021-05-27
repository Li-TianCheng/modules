//
// Created by ltc on 2021/5/24.
//

#include "BufferIo.h"

string BufferIo::read(int socketFd) {
    int recvNum = recv(socketFd, readBuffer, sizeof(readBuffer), 0);
    if (recvNum == 0) {
        return "";
    }
    string context = readBuffer;
    context.resize(recvNum);
    return context;
}

void BufferIo::write(int socketFd, const string& context) {
    int idx = 0;
    while (idx < context.length()) {
        int sendNum = send(socketFd, context.substr(idx, WriteBufferSize).c_str(),
                           context.substr(idx, WriteBufferSize).size(), 0);
        if (sendNum < 0) {
            return;
        }
        idx += sendNum;
    }
}