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
#include "time_system/include/TimeSystem.h"


using std::string;

static const int ReadBufferSize = 256;

class TcpSession {
public:
    TcpSession();
    void write(const string &sendMsg);
    void closeConnection();
    void closeListen();
    string addTicker(int h, int m, int s, int ms);
    string addTimer(int h, int m, int s, int ms);
    void deleteTicker(const string& uuid);
    virtual void sessionInit();
    virtual void sessionClear();
    virtual void handleTickerTimeOut();
    virtual void handleTimerTimeOut();
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
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    char buffer[ReadBufferSize];
};

struct EpollEventArg {
    Time* t;
    TcpSession* session;
    EpollEventArg(Time* t, TcpSession* session) : t(t), session(session) {};
};

struct EpollDeleteArg {
    string uuid;
    TcpSession* session;
    EpollDeleteArg(const string& uuid, TcpSession* session) : uuid(uuid), session(session) {};
};

#endif //NET_TCPSESSION_H
