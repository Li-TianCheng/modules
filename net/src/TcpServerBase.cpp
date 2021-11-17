//
// Created by ltc on 2021/7/23.
//

#include "TcpServerBase.h"

void TcpServerBase::close() {
    auto e = ObjPool::allocate<Event>(EventCloseListener, shared_from_this());
	auto l = listener.lock();
	if (l != nullptr) {
		l->receiveEvent(e);
	}
}

shared_ptr<TcpSession> TcpServerBase::getSession() {
    return nullptr;
}

