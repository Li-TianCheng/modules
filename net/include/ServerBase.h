//
// Created by ltc on 2021/7/23.
//

#ifndef NET_SERVERBASE_H
#define NET_SERVERBASE_H

#include <memory>
#include <sys/epoll.h>
#include "AddressType.h"
#include "log/include/LogSystem.h"
#include "event_system/include/EventSystem.h"

class Session;

class ServerBase: public std::enable_shared_from_this<ServerBase>  {
public:
    virtual shared_ptr<Session> getSession();
    void close();
    virtual ~ServerBase() = default;
private:
    friend class Listener;
protected:
	bool udp;
    int port;
    int serverFd;
    AddressType addressType;
    epoll_event epollEvent;
    sockaddr serverAddress;
    weak_ptr<EventSystem> listener;
};

#endif //NET_SERVERBASE_H
