//
// Created by ltc on 2021/7/23.
//

#include "TcpServerBase.h"

void TcpServerBase::close() {
    auto e = ObjPool::allocate<Event>(EventCloseListen, shared_from_this());
    listener->receiveEvent(e);
}

shared_ptr<TcpSession> TcpServerBase::getSession() {
    return nullptr;
}

