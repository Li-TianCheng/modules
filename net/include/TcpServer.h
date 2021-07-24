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
#include "TcpSession.h"
#include "TcpServerBase.h"

template<typename T>
class TcpServer: public TcpServerBase {
public:
    TcpServer() = default;
    std::shared_ptr<TcpSession> getSession() override;
    ~TcpServer() override = default;
};

template<typename T>
std::shared_ptr<TcpSession> TcpServer<T>::getSession() {
    auto session = ObjPool::allocate<T>();
    session->server = shared_from_this();
    return session;
}


#endif //NET_TCPSERVER_H
