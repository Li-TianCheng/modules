//
// Created by ltc on 2021/7/23.
//

#include "TcpServerBase.h"

void TcpServerBase::close() {
    ::shutdown(serverFd, SHUT_RD);
    ::close(serverFd);
    auto e = ObjPool::allocate<Event>(EventCloseListen, shared_from_this());
    listener->receiveEvent(e);
    LOG(Info, "port:"+std::to_string(port)+" listen end");
}

shared_ptr<TcpSession> TcpServerBase::getSession() {
    return nullptr;
}

