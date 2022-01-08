//
// Created by ltc on 2021/5/28.
//

#include "TcpSession.h"

TcpSession::TcpSession(int bufferChunkSize) : isCloseConnection(false), isWriteDone(true), isClose(false), readNum(0), writeNum(0), readBuffer(bufferChunkSize) {
    len = sizeof(address);
}

void TcpSession::write(shared_ptr<vector<char>> sendMsg, size_t offset, size_t end) {
    if (isCloseConnection || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
    msgLock.lock();
	isWriteDone = false;
    msgQueue.emplace_back(sendMsg, offset, end);
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	msgLock.unlock();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void TcpSession::write(shared_ptr<vector<unsigned char>> sendMsg, size_t offset, size_t end) {
    if (isCloseConnection || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
    msgLock.lock();
	isWriteDone = false;
    msgQueue.emplace_back(sendMsg, offset, end);
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
    msgLock.unlock();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void TcpSession::write(shared_ptr<string> sendMsg, size_t offset, size_t end) {
    if (isCloseConnection || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
	isWriteDone = false;
    msgLock.lock();
    msgQueue.emplace_back(sendMsg, offset, end);
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
    msgLock.unlock();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void TcpSession::write(shared_ptr<char> sendMsg, size_t offset, size_t end) {
	if (isCloseConnection || sendMsg == nullptr) {
		return;
	}
	msgLock.lock();
	isWriteDone = false;
	msgQueue.emplace_back(sendMsg, offset, end);
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	msgLock.unlock();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void TcpSession::write(shared_ptr<unsigned char> sendMsg, size_t offset, size_t end) {
	if (isCloseConnection || sendMsg == nullptr) {
		return;
	}
	msgLock.lock();
	isWriteDone = false;
	msgQueue.emplace_back(sendMsg, offset, end);
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	msgLock.unlock();
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void TcpSession::write(Buffer& buffer) {
	size_t curr = 0;
	for (auto& b : buffer.buffer) {
		if (curr+buffer.bufferChunkSize <= buffer.writeIndex) {
			write(b, 0, buffer.bufferChunkSize);
		} else {
			write(b, 0, buffer.writeIndex-curr);
			break;
		}
		curr += buffer.bufferChunkSize;
	}
}

void TcpSession::closeConnection() {
    isCloseConnection = true;
    auto e = ObjPool::allocate<Event>(EventCloseConnection, shared_from_this());
	auto ep = epoll.lock();
	if (ep != nullptr) {
		ep->receiveEvent(e);
	}
}

void TcpSession::closeListen() {
    auto e = ObjPool::allocate<Event>(EventCloseListener, shared_from_this());
	auto ep = epoll.lock();
	if (ep != nullptr) {
		ep->receiveEvent(e);
	}
}

void TcpSession::deleteSession() {
    auto e = ObjPool::allocate<Event>(EventDeleteSession, shared_from_this());
	auto ep = epoll.lock();
	if (ep != nullptr) {
		ep->receiveEvent(e);
	}
}

void TcpSession::sessionInit() {

}

void TcpSession::sessionClear() {

}

shared_ptr<Time> TcpSession::addTicker(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		auto t = ObjPool::allocate<Time>(h, m, s, ms, ep->shared_from_this());
		auto arg = ObjPool::allocate<EpollEventArg>(t, shared_from_this());
		auto e = ObjPool::allocate<Event>(EventTicker, arg);
		ep->receiveEvent(e);
		return t;
	}
	return nullptr;
}

shared_ptr<Time> TcpSession::addTimer(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		auto t = ObjPool::allocate<Time>(h, m, s, ms, ep->shared_from_this());
		auto arg = ObjPool::allocate<EpollEventArg>(t, shared_from_this());
		auto e = ObjPool::allocate<Event>(EventTimer, arg);
		ep->receiveEvent(e);
		return t;
	}
	return nullptr;
}

void TcpSession::handleTimerTimeOut(shared_ptr<Time> t) {

}

void TcpSession::handleTickerTimeOut(shared_ptr<Time> t) {

}

void TcpSession::handleReadDone(iter pos, size_t n) {

}

void TcpSession::readDone(size_t n) {
    readBuffer.readDone(n);
}

void TcpSession::copy(const iter &begin, size_t n, string& buff) {
    readBuffer.copy(begin, n, buff);
}

void TcpSession::copy(const iter &begin, size_t n, vector<char>& buff) {
    readBuffer.copy(begin, n, buff);
}

void TcpSession::copy(const iter &begin, size_t n, vector<unsigned char>& buff) {
    readBuffer.copy(begin, n, buff);
}

void TcpSession::copy(const iter& begin, size_t n, char* buff) {
	readBuffer.copy(begin, n, buff);
}

void TcpSession::copy(const iter& begin, size_t n, unsigned char* buff) {
	readBuffer.copy(begin, n, buff);
}

void TcpSession::copy(const iter& begin, size_t n, Buffer& buff) {
	readBuffer.copy(begin, n, buff);
}

