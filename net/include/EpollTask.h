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
    void addListener(epoll_event& e);
    void delListener(epoll_event& e);
    void addNewSession(int fd, TcpSession* session);
    void delSession(int fd);
private:
    void init();
    static void cycleTask(void* arg);
    static void handleTickerTimeOut(void* arg);
    static void handleTimerTimeOut(void* arg);
    static void handleTicker(void* arg);
    static void handleDeleteTicker(void* arg);
    static void handleTimer(void* arg);
    static void readTask(void* arg);
    static void writeTask(void* arg);
    static void handleCloseConnection(void * arg);
    static void handleCloseListen(void * arg);
private:
    bool shutdown;
    RwLock rwLock;
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
    while (!sessionManager.empty() || !uuidToFd.empty()){}
    ::close(epollFd);
}

template<typename T> inline
void EpollTask<T>::epollCycle() {
    TaskSystem::addTask(cycleTask, this);
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
void EpollTask<T>::addNewSession(int fd, TcpSession* session) {
    rwLock.wrLock();
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
        TcpSession* session = sessionManager[fd];
        session->sessionClear();
        sessionManager.erase(fd);
        ObjPool::deallocate((T*)session);
    }
    ::close(fd);
    rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTickerTimeOut(void *arg) {
    Time* t = (Time*)arg;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.rdLock();
    if (e->sessionManager.find(e->uuidToFd[t->uuid]) != e->sessionManager.end()) {
        e->sessionManager[e->uuidToFd[t->uuid]]->handleTickerTimeOut();
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTimerTimeOut(void *arg) {
    Time* t = (Time*)arg;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    e->rwLock.wrLock();
    if (e->sessionManager.find(e->uuidToFd[t->uuid]) != e->sessionManager.end()) {
        e->sessionManager[e->uuidToFd[t->uuid]]->handleTimerTimeOut();
    }
    if (e->uuidToFd.find(t->uuid) != e->uuidToFd.end()) {
        e->uuidToFd.erase(t->uuid);
    }
    ObjPool::deallocate((Time*)t);
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTicker(void *arg) {
    TcpSession* session = ((EpollEventArg*)arg)->session;
    Time* t = ((EpollEventArg*)arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    ObjPool::deallocate((EpollEventArg*)arg);
    e->rwLock.wrLock();
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTicker, t);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleDeleteTicker(void *arg) {
    EpollTask<T>* e = (EpollTask<T>*)((EpollDeleteArg*)arg)->epoll;
    e->rwLock.wrLock();
    if (e->uuidToFd.find(((EpollDeleteArg*)arg)->uuid) != e->uuidToFd.end()) {
        e->uuidToFd.erase(((EpollDeleteArg*)arg)->uuid);
    }
    ObjPool::deallocate((EpollDeleteArg*)arg);
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::handleTimer(void *arg) {
    TcpSession* session = ((EpollEventArg*)arg)->session;
    Time* t = ((EpollEventArg*)arg)->t;
    EpollTask<T>* e = (EpollTask<T>*)t->ePtr;
    ObjPool::deallocate((EpollEventArg*)arg);
    e->rwLock.wrLock();
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->uuidToFd[t->uuid] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTimer, t);
    }
    e->rwLock.unlock();
}

template<typename T> inline
void EpollTask<T>::addListener(epoll_event& e) {
    e.events = Read | Err;
    e.data.fd = server->getServerFd();
    epoll_ctl(epollFd, EPOLL_CTL_ADD, server->getServerFd(), &e);
}

template<typename T> inline
void EpollTask<T>::delListener(epoll_event& e) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, server->getServerFd(), &e);
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
void EpollTask<T>::writeTask(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    vector<string> msgs = session->getMsgs();
    for (auto& msg : msgs) {
        int sendNum = send(session->epollEvent.data.fd, msg.data(), msg.size(), 0);
        if (sendNum < 0) {
            session->epollEvent.events = Err|Hup|RdHup|Et|OneShot;
            break;
        }
    }
    if (session->isCloseConnection) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
    }
    session->resetEpollEvent();
}

template<typename T> inline
void EpollTask<T>::cycleTask(void *arg) {
    EpollTask<T>* epoll = (EpollTask<T>*)arg;
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
                    if (event.data.fd == epoll->server->getServerFd()) {
                        TcpSession* session = ObjPool::allocate<T>();
                        int clientFd = accept(epoll->server->getServerFd(), &session->address, &session->len);
                        if (clientFd > 0) {
                            epoll->server->addNewSession(clientFd, session);
                        } else {
                            ObjPool::deallocate((T*)session);
                        }
                    } else {
                        epoll->rwLock.rdLock();
                        TcpSession* session = epoll->sessionManager[event.data.fd];
                        TaskSystem::addTask(readTask, session);
                        epoll->rwLock.unlock();
                    }
                }
                if ((event.events & Write) == Write) {
                    epoll->rwLock.rdLock();
                    TcpSession* session = epoll->sessionManager[event.data.fd];
                    TaskSystem::addTask(writeTask, session);
                    epoll->rwLock.unlock();
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
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
        session->resetEpollEvent();
    }
}

template<typename T> inline
void EpollTask<T>::handleCloseListen(void *arg) {
    TcpSession* session = (TcpSession*)arg;
    ((EpollTask<T>*)session->epoll)->server->close();
}

#endif //NET_EPOLLTASK_H
