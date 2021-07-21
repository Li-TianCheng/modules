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
#include <malloc.h>
#include "AddressType.h"
#include "EpollTask.h"
#include "TcpSession.h"


static const int CheckTime = 1000;
static const int MaxEpollNum = 10;
static const int MaxWaitNum = 500;
static const int IncreaseSessionNum = 1000;
static const int DecreaseAvgNum = IncreaseSessionNum / MaxEpollNum;

template<typename T>
class TcpServer: public EventSystem {
public:
    TcpServer(int port, AddressType addressType);
    void serve();
    bool isRunning();
    void close();
    ~TcpServer() override;
private:
    void addNewSession(shared_ptr<TcpSession> session);
    void cycleInit() override;
    static void serverCycle(shared_ptr<void> arg);
private:
    int serverFd;
    int epollFd;
    int waitTime;
    std::atomic<bool> isClose;
    list<shared_ptr<EpollTask<T>>> epollList;
    shared_ptr<EpollTask<T>> waitCLose;
    epoll_event epollEvent;
    sockaddr serverAddress;
};

template<typename T> inline
TcpServer<T>::TcpServer(int port, AddressType addressType) : isClose(false), waitCLose(nullptr), waitTime(-1) {
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
void TcpServer<T>::addNewSession(shared_ptr<TcpSession> session) {
    if (waitCLose != nullptr && waitCLose->sessionNum+waitCLose->timeToFd.size() == 0) {
        waitCLose->close();
        epollList.remove(waitCLose);
        waitCLose = nullptr;
        std::ostringstream log;
        log << "EpollList decrease, current num:" << epollList.size();
        LOG(Info, log.str());
    }
    int min = INT32_MAX;
    shared_ptr<EpollTask<T>> minIter = nullptr;
    int sum = 0;
    vector<int> snapshot;
    for (auto e : epollList) {
        int num = e->sessionNum;
        snapshot.push_back(num);
        sum += num;
        if (num <= min) {
            min = num;
            minIter = e;
        }
    }
    int second = INT32_MAX;
    shared_ptr<EpollTask<T>> tarIter = nullptr;
    int count = -1;
    for (auto e : epollList) {
        count++;
        if (e == minIter) {
            continue;
        }
        int num = snapshot[count];
        if (num <= second) {
            second = num;
            tarIter = e;
        }
    }
    if (waitCLose != nullptr && epollList.size() > 1) {
        waitCLose = minIter;
        if (session != nullptr) {
            session->epoll = tarIter;
            auto e = ObjPool::allocate<Event>(EventAddSession, session);
            tarIter->receiveEvent(e);
            tarIter->sessionNum++;
        }
        if (second+1 >= IncreaseSessionNum && epollList.size() < MaxEpollNum) {
            waitCLose = nullptr;
        }
    } else {
        if (session != nullptr) {
            session->epoll = minIter;
            auto e = ObjPool::allocate<Event>(EventAddSession, session);
            minIter->receiveEvent(e);
            minIter->sessionNum++;
        }
        if (min+1 >= IncreaseSessionNum && epollList.size() < MaxEpollNum) {
            auto e = ObjPool::allocate<EpollTask<T>>(static_pointer_cast<TcpServer<T>>(shared_from_this()));
            epollList.push_back(e);
            std::ostringstream log;
            log << "EpollList increase, current num:" << epollList.size();
            LOG(Info, log.str());
            e->run();
        } else if (epollList.size() > 1 && sum / epollList.size() < DecreaseAvgNum) {
            waitCLose = minIter;
        }
    }
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
        for (auto& e : epollList) {
            e->close();
        }
        LOG(Info, "listen end");
    }
}

template<typename T> inline
TcpServer<T>::~TcpServer() {
    if (!isClose){
        ::close(serverFd);
    }
    for (auto& e : epollList) {
        e->close();
    }
    while (true){
        bool flag = true;
        for (auto& e : epollList) {
            if (e->isRunning()) {
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
    auto e = ObjPool::allocate<EpollTask<T>>(static_pointer_cast<TcpServer<T>>(shared_from_this()));
    epollList.push_back(e);
    e->run();
    LOG(Info, "listen begin");
}

template<typename T>
void TcpServer<T>::serverCycle(shared_ptr<void> arg) {
    auto server = static_pointer_cast<TcpServer<T>>(arg);
    server->cycleInit();
    while (!server->isClose) {
        epoll_event events[MaxWaitNum];
        int num = epoll_wait(server->epollFd, events, MaxWaitNum, server->waitTime);
        if (num <= 0) {
            if (server->epollList.size() > 1) {
                server->addNewSession(nullptr);
                server->waitTime = CheckTime;
            } else {
                malloc_trim(0);
                server->waitTime = CheckTime;
            }
        } else {
            server->waitTime = 0;
        }
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if ((event.events & Err) == Err) {
                server->close();
            }
            if ((event.events & Read) == Read) {
                auto session = ObjPool::allocate<T>();
                int clientFd = accept(server->serverFd, &session->address, &session->len);
                if (clientFd > 0) {
                    session->epollEvent.data.fd = clientFd;
                    server->addNewSession(session);
                }
            }
        }
    }
}

template<typename T>
void TcpServer<T>::serve() {
    serverCycle(shared_from_this());
}

#endif //NET_TCPSERVER_H
