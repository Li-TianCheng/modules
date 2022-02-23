//
// Created by ltc on 2021/5/28.
//

#ifndef NET_TCPSESSION_H
#define NET_TCPSESSION_H

#include <zconf.h>
#include <sys/socket.h>
#include <string>
#include <sys/epoll.h>
#include "Buffer.h"
#include "ServerBase.h"
#include "time_system/include/Time.h"
#include "time_system/include/TimeSystem.h"
#include "my_pthread/include/SpinLock.h"

class Session: public std::enable_shared_from_this<Session> {
public:
    explicit Session(int bufferChunkSize);
    void write(shared_ptr<vector<char>> sendMsg, size_t offset=0, size_t end=-1);
    void write(shared_ptr<vector<unsigned char>> sendMsg, size_t offset=0, size_t end=-1);
    void write(shared_ptr<string> sendMsg, size_t offset=0, size_t end=-1);
	void write(shared_ptr<char> sendMsg, size_t offset, size_t end);
	void write(shared_ptr<unsigned char> sendMsg, size_t offset, size_t end);
	void write(Buffer& buffer);
    void copy(const iter& begin, size_t n, string& buff);
    void copy(const iter& begin, size_t n, vector<char>& buff);
    void copy(const iter& begin, size_t n, vector<unsigned char>& buff);
	void copy(const iter& begin, size_t n, char* buff);
	void copy(const iter& begin, size_t n, unsigned char* buff);
	void copy(const iter& begin, size_t n, Buffer& buff);
    void closeConnection();
    void closeServer();
    void deleteSession();
    shared_ptr<Time> addTicker(int h, int m, int s, int ms);
	shared_ptr<Time> addTimer(int h, int m, int s, int ms);
    void readDone(size_t n);
    virtual void sessionInit();
    virtual void sessionClear();
    virtual void handleTickerTimeOut(shared_ptr<Time> t);
    virtual void handleTimerTimeOut(shared_ptr<Time> t);
    virtual void handleReadDone(iter pos, size_t n);
    virtual ~Session() = default;
private:
    friend class EpollTask;
    template<typename T>
    friend class Server;
    friend class Listener;
private:
	struct Msg {
		int type;
		size_t offset;
		size_t end;
		shared_ptr<void> msg;
		Msg(shared_ptr<string> msg, size_t offset, size_t end) : type(0), msg(msg), offset(offset), end(end) {
			if (end == -1) {
				this->end = msg->size();
			}
		}
		Msg(shared_ptr<vector<char>> msg, size_t offset, size_t end) : type(1), msg(msg), offset(offset), end(end) {
			if (end == -1) {
				this->end = msg->size();
			}
		}
		Msg(shared_ptr<vector<unsigned char>> msg, size_t offset, size_t end) : type(2), msg(msg), offset(offset), end(end) {
			if (end == -1) {
				this->end = msg->size();
			}
		}
		Msg(shared_ptr<char> msg, size_t offset, size_t end) : type(3), msg(msg), offset(offset), end(end) {

		}
		Msg(shared_ptr<unsigned char> msg, size_t offset, size_t end) : type(4), msg(msg), offset(offset), end(end) {

		}
	};
protected:
    std::atomic<int> isLive;
	int readNum;
	int writeNum;
	SpinLock readLock;
	SpinLock writeLock;
	SpinLock msgLock;
    deque<Msg> msgQueue;
    weak_ptr<EventSystem> epoll;
	weak_ptr<ServerBase> server;
    int epollFd;
    sockaddr address;
    socklen_t len;
    epoll_event epollEvent;
    Buffer readBuffer;
};

#endif //NET_TCPSESSION_H
