//
// Created by ltc on 2021/5/28.
//

#ifndef NET_TCPSERVER_H
#define NET_TCPSERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include "AddressType.h"
#include "EpollTask.h"

static const int EpollNum = 3;
static const int MaxWaitNum = 500;

template<typename T>
class TcpServer {
public:
    TcpServer(int port, AddressType addressType);
    void serve();
    bool isRunning();
    void close();
    ~TcpServer();
private:
    void addNewSession(int fd, const shared_ptr<TcpSession>& session);
    void cycleInit();
    int hash(int fd);
    static void serverCycle(const shared_ptr<void>& arg);
private:
    int serverFd;
    int epollFd;
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
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        throw std::runtime_error("epoll 申请错误");
    }
    epollEvent.data.fd = serverFd;
    epollEvent.events = Read | Err;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &epollEvent);
}

template<typename T> inline
void TcpServer<T>::addNewSession(int fd, const shared_ptr<TcpSession>& session) {
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
        isClose = true;
        ::shutdown(serverFd, SHUT_RD);
        epoll_ctl(epollFd, EPOLL_CTL_DEL, serverFd, &epollEvent);
        ::close(serverFd);
        ::close(epollFd);
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
    for (auto& e : epollList) {
        e.epollCycle();
    }
}

template<typename T>
void TcpServer<T>::serverCycle(const shared_ptr<void> &arg) {
    TcpServer<T>* server = *static_pointer_cast<TcpServer<T>*>(arg);
    server->cycleInit();
    while (!server->isClose) {
        epoll_event events[MaxWaitNum];
        int num = epoll_wait(server->epollFd, events, MaxWaitNum, WaitTime);
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if ((event.events & Err) == Err) {
                server->close();
            }
            if ((event.events & Read) == Read) {
                auto session = ObjPool::allocate<T>();
                int clientFd = accept(server->serverFd, &session->address, &session->len);
                if (clientFd > 0) {
                    server->addNewSession(clientFd, session);
                }
            }
        }
    }
}

template<typename T>
void TcpServer<T>::serve() {
    auto arg = ObjPool::allocate<TcpServer<T>*>(this);
    serverCycle(arg);
}

#endif //NET_TCPSERVER_H
