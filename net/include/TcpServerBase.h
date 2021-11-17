//
// Created by ltc on 2021/7/23.
//

#ifndef NET_TCPSERVERBASE_H
#define NET_TCPSERVERBASE_H

#include <memory>
#include <sys/epoll.h>
#include "AddressType.h"
#include "log/include/LogSystem.h"
#include "event_system/include/EventSystem.h"

class TcpSession;

class TcpServerBase: public std::enable_shared_from_this<TcpServerBase>  {
public:
    virtual shared_ptr<TcpSession> getSession();
    void close();
    virtual ~TcpServerBase() = default;
private:
    friend class Listener;
protected:
    int port;
    int serverFd;
    AddressType addressType;
    epoll_event epollEvent;
    sockaddr serverAddress;
    weak_ptr<EventSystem> listener;
};

#endif //NET_TCPSERVERBASE_H
