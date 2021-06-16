//
// Created by ltc on 2021/5/28.
//

#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include "AddressType.h"
#include "EpollTask.h"

static const int EpollNum = 3;
static const int MaxWaitNum = 5000;

template<typename T>
class TcpServer : public EventSystem {
public:
    TcpServer(int port, AddressType addressType);
    int getServerFd();
    void addNewSession(int fd, TcpSession* session);
    bool isRunning();
    void close();
    void cycleInit() override;
    ~TcpServer() override;
private:
    int hash(int fd);
private:
    int serverFd;
    volatile bool isClose;
    EpollTask<T> epollList[EpollNum];
    epoll_event epollEvent;
    sockaddr serverAddress;
};


template<typename T> inline
TcpServer<T>::TcpServer(int port, AddressType addressType) : isClose(false) {
    serverFd = socket(addressType, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bzero(&serverAddress, sizeof(serverAddress));
    if (addressType == IPV4) {
        ((sockaddr_in*)(&serverAddress))->sin_family = PF_INET;
        ((sockaddr_in*)(&serverAddress))->sin_port = htons(port);
    }else {
        ((sockaddr_in6*)(&serverAddress))->sin6_family = PF_INET6;
        ((sockaddr_in6*)(&serverAddress))->sin6_port = htons(port);
        // TODO 添加IPV6
    }
    int err = bind(serverFd, &serverAddress, sizeof(serverAddress));
    if (err == -1){
        ::close(serverFd);
        throw std::runtime_error("listener创建失败");
    }
    for (auto& e : epollList) {
        e.setServer(this);
    }
    signal(SIGPIPE, SIG_IGN);
    registerEvent(EventEndCycle, nullptr);
}

template<typename T> inline
int TcpServer<T>::getServerFd() {
    return serverFd;
}

template<typename T> inline
void TcpServer<T>::addNewSession(int fd, TcpSession* session) {
    epollList[hash(fd)].addNewSession(fd, session);
}

template<typename T> inline
int TcpServer<T>::hash(int fd) {
    return std::hash<int>()(fd) % EpollNum;
}

template<typename T> inline
bool TcpServer<T>::isRunning() {
    return !isClose;
}

template<typename T> inline
void TcpServer<T>::close() {
    if (!isClose){
        Event* e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
        receiveEvent(e);
        isClose = true;
        ::shutdown(serverFd, SHUT_RD);
        epollList[0].delListener(epollEvent);
        ::close(serverFd);
    }
}

template<typename T> inline
TcpServer<T>::~TcpServer() {
    if (!isClose){
        ::close(serverFd);
    }
    while (true){
        bool flag = true;
        for (auto& e : epollList) {
            if (e.isRunning()) {
                flag = false;
                break;
            }
        }
        if (flag) {
            break;
        }
    }
}

template<typename T> inline
void TcpServer<T>::cycleInit() {
    int err = ::listen(serverFd, MaxWaitNum);
    if (err == -1){
        ::close(serverFd);
        throw std::runtime_error("监听失败");
    }
    epollList[0].addListener(epollEvent);
    for (auto& e : epollList) {
        e.epollCycle();
    }
}

#endif //NET_TCPSERVER_H
