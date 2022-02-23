//
// Created by ltc on 2021/5/28.
//

#include "Session.h"
#include "EpollTask.h"

Session::Session(int bufferChunkSize) : isLive(1), readNum(0), writeNum(0), readBuffer(bufferChunkSize) {
    len = sizeof(address);
}

void Session::write(shared_ptr<vector<char>> sendMsg, size_t offset, size_t end) {
    if (!isLive || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
    msgLock.lock();
    msgQueue.emplace_back(sendMsg, offset, end);
	msgLock.unlock();
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::write(shared_ptr<vector<unsigned char>> sendMsg, size_t offset, size_t end) {
    if (!isLive || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
    msgLock.lock();
    msgQueue.emplace_back(sendMsg, offset, end);
    msgLock.unlock();
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::write(shared_ptr<string> sendMsg, size_t offset, size_t end) {
    if (!isLive || sendMsg == nullptr || (*sendMsg).empty() || offset >= sendMsg->size()) {
        return;
    }
    msgLock.lock();
    msgQueue.emplace_back(sendMsg, offset, end);
    msgLock.unlock();
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::write(shared_ptr<char> sendMsg, size_t offset, size_t end) {
	if (!isLive || sendMsg == nullptr) {
		return;
	}
	msgLock.lock();
	msgQueue.emplace_back(sendMsg, offset, end);
	msgLock.unlock();
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::write(shared_ptr<unsigned char> sendMsg, size_t offset, size_t end) {
	if (!isLive || sendMsg == nullptr) {
		return;
	}
	msgLock.lock();
	msgQueue.emplace_back(sendMsg, offset, end);
	msgLock.unlock();
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::write(Buffer& buffer) {
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

void Session::closeConnection() {
	if (!isLive) {
		return;
	}
	isLive = 0;
	epollEvent.events = EPOLLOUT|EPOLLERR|EPOLLRDHUP|EPOLLHUP|EPOLLET;
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, epollEvent.data.fd, &epollEvent) == -1) {
		epoll_ctl(epollFd, EPOLL_CTL_ADD, epollEvent.data.fd, &epollEvent);
	}
}

void Session::closeServer() {
	auto ep = static_pointer_cast<EpollTask>(epoll.lock());
	if (ep != nullptr) {
		ep->closeServer(server.lock());
	}
}

void Session::deleteSession() {
	auto ep = static_pointer_cast<EpollTask>(epoll.lock());
	if (ep != nullptr) {
		ep->deleteSession(shared_from_this());
	}
}

void Session::sessionInit() {

}

void Session::sessionClear() {

}

shared_ptr<Time> Session::addTicker(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		return static_pointer_cast<EpollTask>(ep)->addTicker(shared_from_this(), h, m, s, ms);
	}
	return nullptr;
}

shared_ptr<Time> Session::addTimer(int h, int m, int s, int ms) {
	auto ep = epoll.lock();
	if (ep != nullptr) {
		return static_pointer_cast<EpollTask>(ep)->addTimer(shared_from_this(), h, m, s, ms);
	}
	return nullptr;
}

void Session::handleTimerTimeOut(shared_ptr<Time> t) {

}

void Session::handleTickerTimeOut(shared_ptr<Time> t) {

}

void Session::handleReadDone(iter pos, size_t n) {

}

void Session::readDone(size_t n) {
    readBuffer.readDone(n);
}

void Session::copy(const iter &begin, size_t n, string& buff) {
    readBuffer.copy(begin, n, buff);
}

void Session::copy(const iter &begin, size_t n, vector<char>& buff) {
    readBuffer.copy(begin, n, buff);
}

void Session::copy(const iter &begin, size_t n, vector<unsigned char>& buff) {
    readBuffer.copy(begin, n, buff);
}

void Session::copy(const iter& begin, size_t n, char* buff) {
	readBuffer.copy(begin, n, buff);
}

void Session::copy(const iter& begin, size_t n, unsigned char* buff) {
	readBuffer.copy(begin, n, buff);
}

void Session::copy(const iter& begin, size_t n, Buffer& buff) {
	readBuffer.copy(begin, n, buff);
}

