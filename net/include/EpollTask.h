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
#include "my_pthread/include/RwLock.h"
#include "TcpSession.h"


using std::unordered_map;
using std::vector;

static const int WaitTime = 10;
static const int EventNum = 2000;

template<typename T>
class TcpServer;

template<typename T>
class EpollTask : public EventSystem {
public:
    EpollTask();
    void setServer(TcpServer<T>* server);
    void epollCycle();
    bool isRunning();
    ~EpollTask() override;
    void addNewSession(int fd, const shared_ptr<TcpSession>& session);
    void delSession(int fd);
private:
    void init();
    static void cycleTask(const shared_ptr<void>& arg);
    static void handleTickerTimeOut(const shared_ptr<void>& arg);
    static void handleTimerTimeOut(const shared_ptr<void>& arg);
    static void handleTicker(const shared_ptr<void>& arg);
    static void handleDeleteTicker(const shared_ptr<void>& arg);
    static void handleTimer(const shared_ptr<void>& arg);
    static void readTask(const shared_ptr<void>& arg);
    static void writeTask(const shared_ptr<void>& arg);
    static void handleCloseConnection(const shared_ptr<void>& arg);
    static void handleCloseListen(const shared_ptr<void>& arg);
private:
    bool shutdown;
    RwLock rwLock;
    int epollFd;
    TcpServer<T>* server;
    unordered_map<int, shared_ptr<TcpSession>> sessionManager;
    unordered_map<string, int> uuidToFd;
};

template<typename T> inline
EpollTask<T>::EpollTask() : shutdown(false) {
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        throw std::runtime_error("epoll 申请错误");
    }
    init();
}

template<typename T> inline
EpollTask<T>::~EpollTask() {
    while (!sessionManager.empty() || !uuidToFd.empty()){}
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
    registerEvent(EventDeleteTicker, handleDeleteTicker);
    registerEvent(EventTicker, handleTicker);
    registerEvent(EventTimer, handleTimer);
}

template<typename T> inline
void EpollTask<T>::addNewSession(int fd, const shared_ptr<TcpSession>& session) {
    rwLock.wrLock();
    int flags = fcntl(fd, 0);
    fcntl(fd, F_SETFL, flags|O_NONBLOCK);
    session->epollFd = epollFd;
    session->epollEvent.events = Read|Err|Hup|RdHup|Et|OneShot;
    session->epollEvent.data.fd = fd;
    session->epoll = this;
    session->sessionInit();
    sessionManager[fd] = session;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &session->epollEvent);
    rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::delSession(int fd) {
    rwLock.wrLock();
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &sessionManager[fd]->epollEvent);
    if (sessionManager.find(fd) != sessionManager.end()) {
        auto session = sessionManager[fd];
        session->sessionClear();
        sessionManager.erase(fd);
    }
    ::close(fd);
    rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTickerTimeOut(const shared_ptr<void>& arg) {
    auto t = static_pointer_cast<Time>(arg);
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.rdLock();
    if (e->sessionManager.find(e->uuidToFd[t->uuid]) != e->sessionManager.end()) {
        e->sessionManager[e->uuidToFd[t->uuid]]->handleTickerTimeOut(t->uuid);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTimerTimeOut(const shared_ptr<void>& arg) {
    auto t = static_pointer_cast<Time>(arg);
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.wrLock();
    if (e->sessionManager.find(e->uuidToFd[t->uuid]) != e->sessionManager.end()) {
        e->sessionManager[e->uuidToFd[t->uuid]]->handleTimerTimeOut(t->uuid);
    }
    if (e->uuidToFd.find(t->uuid) != e->uuidToFd.end()) {
        e->uuidToFd.erase(t->uuid);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTicker(const shared_ptr<void>& arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.wrLock();
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTicker, t);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleDeleteTicker(const shared_ptr<void>& arg) {
    EpollTask<T>* e = (EpollTask<T>*)(static_pointer_cast<EpollDeleteArg>(arg))->epoll;
    e->rwLock.wrLock();
    if (e->uuidToFd.find(static_pointer_cast<EpollDeleteArg>(arg)->uuid) != e->uuidToFd.end()) {
        e->uuidToFd.erase(static_pointer_cast<EpollDeleteArg>(arg)->uuid);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTimer(const shared_ptr<void>& arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.wrLock();
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTimer, t);
    }
    e->rwLock.unlock();
}

template<typename T> inline
bool EpollTask<T>::isRunning() {
    return !shutdown;
}

template<typename T> inline
void EpollTask<T>::readTask(const shared_ptr<void>& arg) {
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
void EpollTask<T>::writeTask(const shared_ptr<void>& arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    session->mutex.lock();
    while (!session->msgQueue.empty()) {
        int offset = 0;
        while (offset != session->msgQueue.front().size()) {
            int sendNum = send(session->epollEvent.data.fd, session->msgQueue.front().data()+offset, session->msgQueue.front().size()-offset, MSG_DONTWAIT);
            if (sendNum <= 0) {
                if (errno == EAGAIN) {
                    session->msgQueue.front() = std::move(session->msgQueue.front().substr(offset));
                    epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
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
            offset += sendNum;
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
void EpollTask<T>::cycleTask(const shared_ptr<void>& arg) {
    EpollTask<T>* epoll = *static_pointer_cast<EpollTask<T>*>(arg);
    epoll->cycleInit();
    while (epoll->server->isRunning() || !epoll->sessionManager.empty() || !epoll->uuidToFd.empty()) {
        epoll->cycleNoBlock();
        epoll_event events[EventNum];
        int num = epoll_wait(epoll->epollFd, events, EventNum, WaitTime);
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if (((event.events & RdHup) == RdHup) || ((event.events & Err) == Err) || ((event.events & Hup) == Hup)) {
                epoll->delSession(event.data.fd);
            } else {
                if ((event.events & Read) == Read) {
                    epoll->rwLock.rdLock();
                    auto session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addTask(readTask, session);
                    epoll->rwLock.unlock();
                }
                if ((event.events & Write) == Write) {
                    epoll->rwLock.rdLock();
                    auto session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addTask(writeTask, session);
                    epoll->rwLock.unlock();
                }
            }
        }
    }
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    epoll->receiveEvent(e);
    epoll->shutdown = true;
}

template<typename T> inline
void EpollTask<T>::setServer(TcpServer<T> *server) {
    this->server = server;
}

template<typename T> inline
void EpollTask<T>::handleCloseConnection(const shared_ptr<void>& arg) {
    auto session = *static_pointer_cast<TcpSession*>(arg);
    if (!session->isWrite) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
        session->resetEpollEvent();
    }
}

template<typename T> inline
void EpollTask<T>::handleCloseListen(const shared_ptr<void>& arg) {
    auto session = *static_pointer_cast<TcpSession*>(arg);
    ((EpollTask<T>*)session->epoll)->server->close();
}

#endif //NET_EPOLLTASK_H
