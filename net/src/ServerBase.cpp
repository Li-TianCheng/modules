//
// Created by ltc on 2021/7/23.
//

#include "ServerBase.h"
#include "Listener.h"

void ServerBase::close() {
	auto l = listener.lock();
	if (l != nullptr) {
		static_pointer_cast<Listener>(l)->closeServer(shared_from_this());
	}
}

shared_ptr<Session> ServerBase::getSession() {
    return nullptr;
}

