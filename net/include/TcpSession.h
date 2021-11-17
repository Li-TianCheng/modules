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

struct Msg {
    int type;
    size_t offset;
    size_t end;
    shared_ptr<void> msg;
    explicit Msg(shared_ptr<string> msg, size_t offset, size_t end) : type(0), msg(msg), offset(offset), end(end) {
        if (end == -1) {
            this->end = msg->size();
        }
    }
    explicit Msg(shared_ptr<vector<char>> msg, size_t offset, size_t end) : type(1), msg(msg), offset(offset), end(end) {
        if (end == -1) {
            this->end = msg->size();
        }
    }
    explicit Msg(shared_ptr<vector<unsigned char>> msg, size_t offset, size_t end) : type(2), msg(msg), offset(offset), end(end) {
        if (end == -1) {
            this->end = msg->size();
        }
    }
};

class TcpSession: public std::enable_shared_from_this<TcpSession> {
public:
    explicit TcpSession(int bufferChunkSize);
    void write(shared_ptr<vector<char>> sendMsg, size_t offset=0, size_t end=-1);
    void write(shared_ptr<vector<unsigned char>> sendMsg, size_t offset=0, size_t end=-1);
    void write(shared_ptr<string> sendMsg, size_t offset=0, size_t end=-1);
    void copy(const iter& begin, size_t n, string& buff);
    void copy(const iter& begin, size_t n, vector<char>& buff);
    void copy(const iter& begin, size_t n, vector<unsigned char>& buff);
    void closeConnection();
    void closeListen();
    void deleteSession();
    shared_ptr<Time> addTicker(int h, int m, int s, int ms);
	shared_ptr<Time> addTimer(int h, int m, int s, int ms);
    void readDone(size_t n);
    virtual void sessionInit();
    virtual void sessionClear();
    virtual void handleTickerTimeOut(shared_ptr<Time> t);
    virtual void handleTimerTimeOut(shared_ptr<Time> t);
    virtual void handleReadDone(iter pos, size_t n);
    virtual ~TcpSession() = default;
private:
    friend class EpollTask;
    template<typename T>
    friend class TcpServer;
    friend class Listener;
protected:
    std::atomic<bool> isCloseConnection;
    std::atomic<bool> isWrite;
    std::atomic<bool> isRead;
    std::atomic<bool> isClose;
    std::atomic<bool> isWriteDone;
    deque<Msg> msgQueue;
    Mutex mutex;
    weak_ptr<EventSystem> epoll;
	weak_ptr<TcpServerBase> server;
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
