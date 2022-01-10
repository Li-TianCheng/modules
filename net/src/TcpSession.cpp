//
// Created by ltc on 2021/5/28.
//

#include "TcpSession.h"
#include "EpollTask.h"

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
	auto ep = static_pointer_cast<EpollTask>(epoll.lock());
	if (ep != nullptr) {
		ep->closeConnection(shared_from_this());
	}
}

void TcpSession::closeServer() {
	auto ep = static_pointer_cast<EpollTask>(epoll.lock());
	if (ep != nullptr) {
		ep->closeServer(server.lock());
	}
}

void TcpSession::deleteSession() {
	auto ep = static_pointer_cast<EpollTask>(epoll.lock());
	if (ep != nullptr) {
		ep->deleteSession(shared_from_this());
	}
}

void TcpSession::sessionInit() {

}

void TcpSession::sessionClear() {

}

shared_ptr<Time> TcpSession::addTicker(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		return static_pointer_cast<EpollTask>(ep)->addTicker(shared_from_this(), h, m, s, ms);
	}
	return nullptr;
}

shared_ptr<Time> TcpSession::addTimer(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		return static_pointer_cast<EpollTask>(ep)->addTimer(shared_from_this(), h, m, s, ms);
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

