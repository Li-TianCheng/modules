//
// Created by ltc on 2021/7/23.
//

#include "TcpServerBase.h"
#include "Listener.h"

void TcpServerBase::close() {
	auto l = listener.lock();
	if (l != nullptr) {
		static_pointer_cast<Listener>(l)->closeServer(shared_from_this());
	}
}

shared_ptr<TcpSession> TcpServerBase::getSession() {
    return nullptr;
}

