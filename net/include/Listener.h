//
// Created by ltc on 2021/7/23.
//

#ifndef NET_LISTENER_H
#define NET_LISTENER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <malloc.h>
#include "EpollEventType.h"
#include "AddressType.h"
#include "TcpServerBase.h"
#include "log/include/LogSystem.h"
#include "event_system/include/EventSystem.h"
#include "AddressType.h"
#include "EpollTask.h"
#include "TcpSession.h"

static const int MaxWaitNum = 500;
static const int CheckTime = 1000;
static const int MaxEpollNum = 10;
static const int IncreaseSessionNum = 1000;
static const int DecreaseAvgNum = IncreaseSessionNum / MaxEpollNum;

class Listener: public EventSystem {
public:
    Listener();
    void registerListener(int port, AddressType addressType, shared_ptr<TcpServerBase> server);
    void listen();
    ~Listener() override;
private:
    static void handleCloseListen(shared_ptr<void> arg);
    void cycleInit() override;
    void addNewSession(shared_ptr<TcpSession> session);
private:
    int epollFd;
    int waitTime;
    unordered_map<int, shared_ptr<TcpServerBase>> listenMap;
    list<shared_ptr<EpollTask>> epollList;
    shared_ptr<EpollTask> waitCLose;
};



#endif //NET_LISTENER_H
