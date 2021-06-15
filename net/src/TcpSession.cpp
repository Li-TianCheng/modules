//
// Created by ltc on 2021/5/28.
//

#include "TcpSession.h"

TcpSession::TcpSession() : isCloseConnection(false), isWrite(false), isRead(false) {
    len = sizeof(address);
}

void TcpSession::write(const string &sendMsg) {
    mutex.lock();
    msgQueue.push(sendMsg);
    isWrite = true;
    epollEvent.events |= Write;
    mutex.unlock();
    epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent);
}

void TcpSession::closeConnection() {
    isCloseConnection = true;
    Event* e = ObjPool::allocate<Event>(EventCloseConnection, this);
    epoll->receiveEvent(e);
}

void TcpSession::closeListen() {
    Event* e = ObjPool::allocate<Event>(EventCloseListen, this);
    epoll->receiveEvent(e);
}

vector<string> TcpSession::getMsgs() {
    vector<string> msgs;
    mutex.lock();
    while (!msgQueue.empty()) {
        string temp = msgQueue.front();
        msgs.push_back(temp);
        msgQueue.pop();
    }
    isWrite = false;
    epollEvent.events ^= Write;
    mutex.unlock();
    return msgs;
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
    Time* t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    EpollEventArg* arg = ObjPool::allocate<EpollEventArg>(t, this);
    Event* e = ObjPool::allocate<Event>(EventTicker, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

string TcpSession::addTimer(int h, int m, int s, int ms) {
    Time* t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    EpollEventArg* arg = ObjPool::allocate<EpollEventArg>(t, this);
    Event* e = ObjPool::allocate<Event>(EventTimer, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

void TcpSession::deleteTicker(string &uuid) {
    TimeSystem::deleteTicker(uuid);
    EpollDeleteArg* arg = ObjPool::allocate<EpollDeleteArg>(uuid, epoll);
    Event* e = ObjPool::allocate<Event>(EventDeleteTicker, arg);
    epoll->receiveEvent(e);
}

void TcpSession::handleTimerTimeOut(const string& uuid) {

}

void TcpSession::handleTickerTimeOut(const string& uuid) {

}

