//
// Created by ltc on 2021/5/28.
//

#ifndef NET_TCPSESSION_H
#define NET_TCPSESSION_H

#include <zconf.h>
#include <sys/socket.h>
#include <string>
#include "EpollEventType.h"
#include "time_system/include/Time.h"

using std::string;

static const int ReadBufferSize = 256;

class TcpSession {
public:
    TcpSession();
    void write(const string &sendMsg);
    void closeConnection();
    void closeListen();
    virtual void handleTimeOut() = 0;
    virtual void handleReadDone(const string& recvMsg) = 0;
    virtual ~TcpSession() = default;
private:
    vector<string> getMsgs();
    void resetEpollEvent();
    template<typename T>
    friend class EpollTask;
private:
    volatile bool isCloseConnection;
    volatile bool isWrite;
    volatile bool isRead;
    queue<string> msgQueue;
    Mutex mutex;
    EventSystem* epoll;
    int epollFd;
    Time* time;
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    char buffer[ReadBufferSize];
};


#endif //NET_TCPSESSION_H
