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
#include "TcpServerBase.h"
#include "time_system/include/Time.h"
#include "time_system/include/TimeSystem.h"

static const int AppendSize = 1024;

struct Msg {
    int type;
    size_t offset;
    shared_ptr<void> msg;
    explicit Msg(shared_ptr<string> msg) : type(0), offset(0) {
        this->msg = msg;
    }
    explicit Msg(shared_ptr<vector<char>> msg) : type(1), offset(0) {
        this->msg = msg;
    }
    explicit Msg(shared_ptr<vector<unsigned char>> msg) : type(2), offset(0) {
        this->msg = msg;
    }
};

class TcpSession: public std::enable_shared_from_this<TcpSession> {
public:
    TcpSession();
    void write(shared_ptr<vector<char>> sendMsg);
    void write(shared_ptr<vector<unsigned char>> sendMsg);
    void write(shared_ptr<string> sendMsg);
    void closeConnection();
    void closeListen();
    void deleteSession();
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
    friend class EpollTask;
    template<typename T>
    friend class TcpServer;
    friend class Listener;
protected:
    std::shared_ptr<TcpServerBase> server;
    std::atomic<bool> isCloseConnection;
    std::atomic<bool> isWrite;
    std::atomic<bool> isRead;
    std::atomic<bool> isWriteDone;
    deque<Msg> msgQueue;
    Mutex mutex;
    shared_ptr<EventSystem> epoll;
    int epollFd;
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    Buffer readBuffer;
};

struct EpollEventArg {
    shared_ptr<Time> t;
    shared_ptr<TcpSession> session;
    EpollEventArg(shared_ptr<Time> t, shared_ptr<TcpSession> session) : t(t), session(session) {};
};

#endif //NET_TCPSESSION_H
