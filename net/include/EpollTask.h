//
// Created by ltc on 2021/5/28.
//

#ifndef NET_EPOLLTASK_H
#define NET_EPOLLTASK_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include "EpollEventType.h"
#include "time_system/include/TimeSystem.h"
#include "event_system/include/EventSystem.h"
#include "task_system/include/TaskSystem.h"
#include "TcpSession.h"


using std::unordered_map;
using std::vector;

static const int WaitTime = 1;
static const int EpollEventNum = 5000;

template<typename T>
class TcpServer;

template<typename T>
class EpollTask : public EventSystem {
public:
    explicit EpollTask(TcpServer<T>* server);
    bool isRunning();
    ~EpollTask() override;
    void addNewSession(shared_ptr<TcpSession> session);
    void delSession(int fd);
    void close();
private:
    friend class TcpServer<T>;
    void init();
    void epollCycle();
    static void cycleTask(shared_ptr<void> arg);
    static void handleTickerTimeOut(shared_ptr<void> arg);
    static void handleTimerTimeOut(shared_ptr<void> arg);
    static void handleTicker(shared_ptr<void> arg);
    static void handleTimer(shared_ptr<void> arg);
    static void readTask(shared_ptr<void> arg);
    static void writeTask(shared_ptr<void> arg);
    static void handleCloseConnection(shared_ptr<void> arg);
    static void handleCloseListen(shared_ptr<void> arg);
    static void handleAddSession(shared_ptr<void> arg);
private:
    std::atomic<bool> needClose;
    std::atomic<bool> running;
    std::atomic<int> sessionNum;
    std::atomic<int> uuidNum;
    int epollFd;
    TcpServer<T>* server;
    unordered_map<int, shared_ptr<TcpSession>> sessionManager;
    unordered_map<string, int> uuidToFd;
};

template<typename T> inline
EpollTask<T>::EpollTask(TcpServer<T>* server) : running(false), server(server), needClose(false), sessionNum(0), uuidNum(0) {
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        throw std::runtime_error("epoll 申请错误");
    }
    init();
    epollCycle();
}

template<typename T> inline
EpollTask<T>::~EpollTask() {
    while (running){
        close();
    }
    ::close(epollFd);
}

template<typename T> inline
void EpollTask<T>::epollCycle() {
    auto arg = ObjPool::allocate<EpollTask<T>*>(this);
    TaskSystem::addTask(cycleTask, arg);
}

template<typename T> inline
void EpollTask<T>::init() {
    registerEvent(EventEndCycle, nullptr);
    registerEvent(EventTickerTimeOut, handleTickerTimeOut);
    registerEvent(EventTimerTimeOut, handleTimerTimeOut);
    registerEvent(EventCloseListen, handleCloseListen);
    registerEvent(EventCloseConnection, handleCloseConnection);
    registerEvent(EventTicker, handleTicker);
    registerEvent(EventTimer, handleTimer);
    registerEvent(EventAddSession, handleAddSession);
}

template<typename T>
void EpollTask<T>::handleAddSession(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    ((EpollTask<T>*)session->epoll)->addNewSession(session);
}

template<typename T> inline
void EpollTask<T>::addNewSession(shared_ptr<TcpSession> session) {
    int fd = session->epollEvent.data.fd;
    int flags = fcntl(fd, 0);
    fcntl(fd, F_SETFL, flags|O_NONBLOCK);
    session->epollFd = epollFd;
    session->epollEvent.events = Read|Err|Hup|RdHup|Et|OneShot;
    session->sessionInit();
    sessionManager[fd] = session;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &session->epollEvent);
}

template<typename T> inline
void EpollTask<T>::delSession(int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &sessionManager[fd]->epollEvent);
    if (sessionManager.find(fd) != sessionManager.end()) {
        auto session = sessionManager[fd];
        session->sessionClear();
        sessionManager.erase(fd);
        sessionNum--;
    }
    ::close(fd);
}

template<typename T> inline
void EpollTask<T>::handleTickerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<Time>(arg);
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    if (e->uuidToFd.find(t->uuid) != e->uuidToFd.end()) {
        int fd = e->uuidToFd[t->uuid];
        if (e->sessionManager.find(fd) != e->sessionManager.end()) {
            e->sessionManager[fd]->handleTimerTimeOut(t->uuid);
        } else {
            TimeSystem::deleteTicker(t->uuid);
            e->uuidToFd.erase(t->uuid);
            e->uuidNum--;
        }
    }
}

template<typename T> inline
void EpollTask<T>::handleTimerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<Time>(arg);
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    if (e->uuidToFd.find(t->uuid) != e->uuidToFd.end()) {
        int fd = e->uuidToFd[t->uuid];
        if (e->sessionManager.find(fd) != e->sessionManager.end()) {
            e->sessionManager[fd]->handleTimerTimeOut(t->uuid);
        }
        e->uuidToFd.erase(t->uuid);
        e->uuidNum--;
    }
}

template<typename T> inline
void EpollTask<T>::handleTicker(shared_ptr<void> arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        e->uuidNum++;
        TimeSystem::receiveEvent(EventTicker, t);
    }
}

template<typename T> inline
void EpollTask<T>::handleTimer(shared_ptr<void> arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        e->uuidNum++;
        TimeSystem::receiveEvent(EventTimer, t);
    }
}

template<typename T> inline
bool EpollTask<T>::isRunning() {
    return running;
}

template<typename T> inline
void EpollTask<T>::readTask(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    string readMsg;
    session->isRead = true;
    while (true) {
        int recvNum = recv(session->epollEvent.data.fd, session->buffer, sizeof(session->buffer), MSG_DONTWAIT);
        if (recvNum <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break;
            }
            session->epollEvent.events = Err|Hup|RdHup|Et|OneShot;
            session->isRead = false;
            session->resetEpollEvent();
            return;
        }
        string temp = session->buffer;
        temp.resize(recvNum);
        readMsg += temp;
    }
    session->handleReadDone(readMsg);
    session->isRead = false;
    session->resetEpollEvent();
}

template<typename T> inline
void EpollTask<T>::writeTask(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    session->mutex.lock();
    while (!session->msgQueue.empty()) {
        while (session->msgQueue.front().offset != session->msgQueue.front().msg.size()) {
            int sendNum = send(session->epollEvent.data.fd, session->msgQueue.front().msg.data()+session->msgQueue.front().offset, session->msgQueue.front().msg.size()-session->msgQueue.front().offset, MSG_DONTWAIT);
            if (sendNum <= 0) {
                if (errno == EAGAIN) {
                    session->isWrite = false;
                    session->resetEpollEvent();
                    session->mutex.unlock();
                    return;
                } else {
                    session->epollEvent.events = Err|Hup|RdHup|Et|OneShot;
                    session->isWrite = false;
                    session->resetEpollEvent();
                    session->mutex.unlock();
                    return;
                }
            }
            session->msgQueue.front().offset += sendNum;
        }
        session->msgQueue.pop();
    }
    session->isWrite = false;
    session->epollEvent.events &= ~Write;
    session->mutex.unlock();
    if (session->isCloseConnection) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
    }
    session->resetEpollEvent();
}

template<typename T> inline
void EpollTask<T>::cycleTask(shared_ptr<void> arg) {
    EpollTask<T>* epoll = *static_pointer_cast<EpollTask<T>*>(arg);
    epoll->running = true;
    epoll->cycleInit();
    while (!epoll->needClose || epoll->sessionNum + epoll->uuidNum != 0) {
        epoll->cycleNoBlock(-1);
        epoll_event events[EpollEventNum];
        int num = epoll_wait(epoll->epollFd, events, EpollEventNum, WaitTime);
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if (((event.events & RdHup) == RdHup) || ((event.events & Err) == Err) || ((event.events & Hup) == Hup)) {
                epoll->delSession(event.data.fd);
            } else {
                if ((event.events & Read) == Read) {
                    auto session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addTask(readTask, session);
                }
                if ((event.events & Write) == Write) {
                    auto session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addPriorityTask(writeTask, session);
                }
            }
        }
    }
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    epoll->receiveEvent(e);
    epoll->running = false;
}

template<typename T> inline
void EpollTask<T>::handleCloseConnection(shared_ptr<void> arg) {
    auto session = *static_pointer_cast<TcpSession*>(arg);
    session->mutex.lock();
    if (session->msgQueue.empty()) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
        session->resetEpollEvent();
    }
    session->mutex.unlock();
}

template<typename T> inline
void EpollTask<T>::handleCloseListen(shared_ptr<void> arg) {
    auto session = *static_pointer_cast<TcpSession*>(arg);
    ((EpollTask<T>*)session->epoll)->server->close();
}

template<typename T>
void EpollTask<T>::close() {
    needClose = true;
}

#endif //NET_EPOLLTASK_H
