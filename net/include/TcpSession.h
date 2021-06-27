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

struct Msg {
    string msg;
    int offset;
    explicit Msg(string&& msg) : msg(std::forward<string>(msg)), offset(0) {}
};

class TcpSession {
public:
    TcpSession();
    void write(string&& sendMsg);
    void closeConnection();
    void closeListen();
    string addTicker(int h, int m, int s, int ms);
    string addTimer(int h, int m, int s, int ms);
    virtual void sessionInit();
    virtual void sessionClear();
    virtual void handleTickerTimeOut(const string& uuid);
    virtual void handleTimerTimeOut(const string& uuid);
    virtual void handleReadDone(const string& recvMsg);
    virtual ~TcpSession() = default;
private:
    void resetEpollEvent();
    template<typename T>
    friend class EpollTask;
    template<typename T>
    friend class TcpServer;
protected:
    volatile bool isCloseConnection;
    volatile bool isWrite;
    volatile bool isRead;
    queue<Msg> msgQueue;
    Mutex mutex;
    EventSystem* epoll;
    int epollFd;
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    char buffer[ReadBufferSize];
};

struct EpollEventArg {
    shared_ptr<Time> t;
    TcpSession* session;
    EpollEventArg(const shared_ptr<Time>& t, TcpSession* session) : t(t), session(session) {};
};

#endif //NET_TCPSESSION_H
