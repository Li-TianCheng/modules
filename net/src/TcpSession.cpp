//
// Created by ltc on 2021/5/28.
//

#include "TcpSession.h"

TcpSession::TcpSession() : isCloseConnection(false), isWrite(false), isRead(false) {
    len = sizeof(address);
}

void TcpSession::write(string&& sendMsg) {
    mutex.lock();
    msgQueue.emplace(std::forward<string>(sendMsg));
    isWrite = true;
    epollEvent.events |= Write;
    mutex.unlock();
    epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent);
}

void TcpSession::closeConnection() {
    isCloseConnection = true;
    auto arg = ObjPool::allocate<TcpSession*>(this);
    auto e = ObjPool::allocate<Event>(EventCloseConnection, arg);
    epoll->receiveEvent(e);
}

void TcpSession::closeListen() {
    auto arg = ObjPool::allocate<TcpSession*>(this);
    auto e = ObjPool::allocate<Event>(EventCloseListen, arg);
    epoll->receiveEvent(e);
}

void TcpSession::resetEpollEvent() {
    if (isRead || isWrite) {
        return;
    }
    epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent);
}

void TcpSession::sessionInit() {

}

void TcpSession::sessionClear() {

}

string TcpSession::addTicker(int h, int m, int s, int ms) {
    auto t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    auto arg = ObjPool::allocate<EpollEventArg>(t, this);
    auto e = ObjPool::allocate<Event>(EventTicker, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

string TcpSession::addTimer(int h, int m, int s, int ms) {
    auto t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    auto arg = ObjPool::allocate<EpollEventArg>(t, this);
    auto e = ObjPool::allocate<Event>(EventTimer, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

void TcpSession::deleteTicker(string &uuid) {
    TimeSystem::deleteTicker(uuid);
    auto arg = ObjPool::allocate<EpollDeleteArg>(uuid, epoll);
    auto e = ObjPool::allocate<Event>(EventDeleteTicker, arg);
    epoll->receiveEvent(e);
}

void TcpSession::handleTimerTimeOut(const string& uuid) {

}

void TcpSession::handleTickerTimeOut(const string& uuid) {

}

void TcpSession::handleReadDone(const string &recvMsg) {

}

