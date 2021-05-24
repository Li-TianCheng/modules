//
// Created by ltc on 2021/5/24.
//

#ifndef NET_BUFFERIO_H
#define NET_BUFFERIO_H

#include <string>
#include <sys/socket.h>

using std::string;

static const int readBufferSize = 3*1024;
static const int writeBufferSize = 3*1024;

class BufferIo {
public:
    string read(int socketFd);
    void write(int socketFd, const string& context);
private:
    char readBuffer[readBufferSize];
};

#endif //NET_BUFFERIO_H
