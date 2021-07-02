//
// Created by ltc on 2021/5/28.
//

#ifndef NET_TCPSESSION_H
#define NET_TCPSESSION_H

#include <zconf.h>
#include <sys/socket.h>
#include <string>
#include "EpollEventType.h"
#include "Buffer.h"
#include "time_system/include/Time.h"
#include "time_system/include/TimeSystem.h"

struct Msg {
    vector<char> msg;
    string strMsg;
    size_t offset;
    explicit Msg(vector<char>&& msg) : msg(std::forward<vector<char>>(msg)), offset(0) {}
    explicit Msg(string&& strMsg) : strMsg(std::forward<string>(strMsg)), offset(0) {}
};

class TcpSession {
public:
    TcpSession();
    void write(vector<char>&& sendMsg);
    void write(string&& sendMsg);
    void closeConnection();
    void closeListen();
    const string& addTicker(int h, int m, int s, int ms);
    const string& addTimer(int h, int m, int s, int ms);
    void readDone(size_t n);
    virtual void sessionInit();
    virtual void sessionClear();
    virtual void handleTickerTimeOut(const string& uuid);
    virtual void handleTimerTimeOut(const string& uuid);
    virtual void handleReadDone(iter pos, size_t n);
    virtual ~TcpSession() = default;
private:
    void resetEpollEvent();
    template<typename T>
    friend class EpollTask;
    template<typename T>
    friend class TcpServer;
protected:
    std::atomic<bool> isCloseConnection;
    std::atomic<bool> isWrite;
    std::atomic<bool> isRead;
    queue<Msg> msgQueue;
    Mutex mutex;
    EventSystem* epoll;
    int epollFd;
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    Buffer readBuffer;
};

struct EpollEventArg {
    shared_ptr<Time> t;
    TcpSession* session;
    EpollEventArg(shared_ptr<Time> t, TcpSession* session) : t(t), session(session) {};
};

#endif //NET_TCPSESSION_H
