//
// Created by ltc on 2021/5/28.
//

#include "TcpSession.h"

TcpSession::TcpSession() : isCloseConnection(false), isWrite(false), isRead(false) {
    len = sizeof(address);
}

void TcpSession::write(shared_ptr<vector<char>> sendMsg) {
    if (isCloseConnection || sendMsg == nullptr || (*sendMsg).empty()) {
        return;
    }
    mutex.lock();
    if (!msgQueue.empty() && sendMsg->size() <= AppendSize) {
        if (msgQueue.back().type == 0){
            shared_ptr<string> msg = static_pointer_cast<string>(msgQueue.back().msg);
            msg->reserve(msg->size()+sendMsg->size());
            msg->insert(msg->end(), sendMsg->data(), sendMsg->data()+sendMsg->size());
        }
        if (msgQueue.back().type == 1) {
            shared_ptr<vector<char>> msg = static_pointer_cast<vector<char>>(msgQueue.back().msg);
            msg->reserve(msg->size()+sendMsg->size());
            msg->insert(msg->end(), sendMsg->data(), sendMsg->data()+sendMsg->size());
        }
    } else {
        msgQueue.emplace_back(sendMsg);
    }
    epollEvent.events |= Write;
    mutex.unlock();
    epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent);
}

void TcpSession::write(shared_ptr<string> sendMsg) {
    if (isCloseConnection || sendMsg == nullptr || (*sendMsg).empty()) {
        return;
    }
    mutex.lock();
    if (!msgQueue.empty() && sendMsg->size() <= AppendSize) {
        if (msgQueue.back().type == 0){
            shared_ptr<string> msg = static_pointer_cast<string>(msgQueue.back().msg);
            msg->reserve(msg->size()+sendMsg->size());
            msg->insert(msg->end(), sendMsg->data(), sendMsg->data()+sendMsg->size());
        }
        if (msgQueue.back().type == 1) {
            shared_ptr<vector<char>> msg = static_pointer_cast<vector<char>>(msgQueue.back().msg);
            msg->reserve(msg->size()+sendMsg->size());
            msg->insert(msg->end(), sendMsg->data(), sendMsg->data()+sendMsg->size());
        }
    } else {
        msgQueue.emplace_back(sendMsg);
    }
    epollEvent.events |= Write;
    mutex.unlock();
    epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent);
}

void TcpSession::closeConnection() {
    isCloseConnection = true;
    mutex.lock();
    if (msgQueue.empty()) {
        auto arg = ObjPool::allocate<TcpSession*>(this);
        auto e = ObjPool::allocate<Event>(EventCloseConnection, arg);
        epoll->receiveEvent(e);
    }
    mutex.unlock();
}

void TcpSession::closeListen() {
    auto arg = ObjPool::allocate<TcpSession*>(this);
    auto e = ObjPool::allocate<Event>(EventCloseListen, arg);
    epoll->receiveEvent(e);
}

void TcpSession::sessionInit() {

}

void TcpSession::sessionClear() {

}

const string& TcpSession::addTicker(int h, int m, int s, int ms) {
    auto t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    auto arg = ObjPool::allocate<EpollEventArg>(t, this);
    auto e = ObjPool::allocate<Event>(EventTicker, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

const string& TcpSession::addTimer(int h, int m, int s, int ms) {
    auto t = ObjPool::allocate<Time>(h, m, s, ms, epoll);
    auto arg = ObjPool::allocate<EpollEventArg>(t, this);
    auto e = ObjPool::allocate<Event>(EventTimer, arg);
    epoll->receiveEvent(e);
    return t->uuid;
}

void TcpSession::handleTimerTimeOut(const string& uuid) {

}

void TcpSession::handleTickerTimeOut(const string& uuid) {

}

void TcpSession::handleReadDone(iter pos, size_t n) {

}

void TcpSession::readDone(size_t n) {
    readBuffer.readDone(n);
}

