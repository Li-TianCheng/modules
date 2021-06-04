//
// Created by ltc on 2021/5/28.
//

#ifndef NET_EPOLLTASK_H
#define NET_EPOLLTASK_H

#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include "EpollEventType.h"
#include "time_system/include/TimeSystem.h"
#include "event_system/include/EventSystem.h"
#include "task_system/include/TaskSystem.h"
#include "my_pthread/include/Mutex.h"
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
    void addListener(epoll_event& e);
    void addNewSession(int fd, TcpSession* session);
    void delSession(int fd);
private:
    void init();
    static void cycleTask(void* arg);
    static void handleTimeOut(void* arg);
    static void readTask(void* arg);
    static void writeTask(void* arg);
    static void handleCloseConnection(void * arg);
    static void handleCloseListen(void * arg);
private:
    bool shutdown;
    Mutex mutex;
    int epollFd;
    TcpServer<T>* server;
    unordered_map<int, TcpSession*> sessionManager;
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
    while (!sessionManager.empty()){}
    ::close(epollFd);
}

template<typename T> inline
void EpollTask<T>::epollCycle() {
    TaskSystem::addTask(cycleTask, this);
}

template<typename T> inline
void EpollTask<T>::init() {
    registerEvent(EventEndCycle, nullptr);
    registerEvent(EventTickerTimeOut, handleTimeOut);
    registerEvent(EventCloseListen, handleCloseListen);
    registerEvent(EventCloseConnection, handleCloseConnection);
}

template<typename T> inline
void EpollTask<T>::addNewSession(int fd, TcpSession* session) {
    mutex.lock();
    session->epollFd = epollFd;
    session->epollEvent.events = Read|Err|RdHup|Et|OneShot;
    session->epollEvent.data.fd = fd;
    session->epoll = this;
    Time* t = ObjPool::allocate<Time>(0, 0, 1, 0, this);
    string uuid = TimeSystem::receiveEvent(EventTicker, t);
    session->time = t;
    uuidToFd[uuid] = fd;
    sessionManager[fd] = session;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &session->epollEvent);
    mutex.unlock();
}

template<typename T> inline
void EpollTask<T>::delSession(int fd) {
    mutex.lock();
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &sessionManager[fd]->epollEvent);
    uuidToFd.erase(sessionManager[fd]->time->uuid);
    TimeSystem::deleteTicker(sessionManager[fd]->time->uuid);
    TcpSession* session = sessionManager[fd];
    sessionManager.erase(fd);
    ObjPool::deallocate((T*)session);
    ::close(fd);
    mutex.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTimeOut(void *arg) {
    Time* t = (Time*)arg;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->mutex.lock();
    if (e->sessionManager.find(e->uuidToFd[t->uuid]) != e->sessionManager.end()) {
        e->sessionManager[e->uuidToFd[t->uuid]]->handleTimeOut();
    }
    e->mutex.unlock();
}

template<typename T> inline
void EpollTask<T>::addListener(epoll_event& e) {
    e.events = Read | Err;
    e.data.fd = server->getServerFd();
    epoll_ctl(epollFd, EPOLL_CTL_ADD, server->getServerFd(), &e);
}

template<typename T> inline
bool EpollTask<T>::isRunning() {
    return !shutdown;
}

template<typename T> inline
void EpollTask<T>::readTask(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    string readMsg;
    session->isRead = true;
    while (true) {
        int recvNum = recv(session->epollEvent.data.fd, session->buffer, sizeof(session->buffer), MSG_DONTWAIT);
        if (recvNum <= 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                break;
            }
            session->epollEvent.events = Err|RdHup|Et|OneShot;
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
void EpollTask<T>::writeTask(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    vector<string> msgs = session->getMsgs();
    for (auto& msg : msgs) {
        int sendNum = send(session->epollEvent.data.fd, msg.data(), msg.size(), 0);
        if (sendNum < 0) {
            session->epollEvent.events = Err|RdHup|Et|OneShot;
            break;
        }
    }
    if (session->isCloseConnection) {
        ((EpollTask<T>*)session->epoll)->delSession(session->epollEvent.data.fd);
    }
    session->resetEpollEvent();
}

template<typename T> inline
void EpollTask<T>::cycleTask(void *arg) {
    EpollTask<T>* epoll = (EpollTask<T>*)arg;
    while (epoll->server->isRunning() || !epoll->sessionManager.empty()) {
        epoll->cycleNoBlock();
        epoll_event events[EventNum];
        int num = epoll_wait(epoll->epollFd, events, EventNum, WaitTime);
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if (((event.events & RdHup) == RdHup) || ((event.events & Err) == Err)) {
                epoll->delSession(event.data.fd);
            } else {
                if ((event.events & Read) == Read) {
                    if (event.data.fd == epoll->server->getServerFd()) {
                        TcpSession* session = ObjPool::allocate<T>();
                        int clientFd = accept(epoll->server->getServerFd(), &session->address, &session->len);
                        if (clientFd > 0) {
                            epoll->server->addNewSession(clientFd, session);
                        } else {
                            ObjPool::deallocate((T*)session);
                        }
                    } else {
                        epoll->mutex.lock();
                        TcpSession* session = epoll->sessionManager[event.data.fd];
                        TaskSystem::addTask(readTask, session);
                        epoll->mutex.unlock();
                    }
                }
                if ((event.events & Write) == Write) {
                    epoll->mutex.lock();
                    TcpSession* session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addTask(writeTask, session);
                    epoll->mutex.unlock();
                }
            }
        }
    }
    Event* e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    epoll->receiveEvent(e);
    epoll->shutdown = true;
}

template<typename T> inline
void EpollTask<T>::setServer(TcpServer<T> *server) {
    this->server = server;
}

template<typename T> inline
void EpollTask<T>::handleCloseConnection(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    if (!session->isWrite) {
        ((EpollTask<T>*)session->epoll)->delSession(session->epollEvent.data.fd);
    }
}

template<typename T> inline
void EpollTask<T>::handleCloseListen(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    ((EpollTask<T>*)session->epoll)->server->close();
}

#endif //NET_EPOLLTASK_H
