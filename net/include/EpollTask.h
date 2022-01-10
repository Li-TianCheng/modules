//
// Created by ltc on 2021/5/28.
//

#ifndef NET_EPOLLTASK_H
#define NET_EPOLLTASK_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <sstream>
#include <sys/epoll.h>
#include "time_system/include/TimeSystem.h"
#include "event_system/include/EventSystem.h"
#include "task_system/include/TaskSystem.h"
#include "log/include/LogSystem.h"
#include "TcpSession.h"


using std::unordered_map;
using std::vector;

static const int EpollEventNum = 5000;

class EpollTask : public EventSystem {
public:
    explicit EpollTask(int epollEventNum);
    bool isRunning();
    void run();
	shared_ptr<Time> addTicker(shared_ptr<TcpSession> session, int h, int m, int s, int ms);
	shared_ptr<Time> addTimer(shared_ptr<TcpSession> session, int h, int m, int s, int ms);
	void deleteSession(shared_ptr<TcpSession> session);
	void closeServer(shared_ptr<TcpServerBase> server);
    ~EpollTask() override;
private:
    friend class Listener;
    void init();
    void addNewSession(shared_ptr<TcpSession> session);
    void deleteSession(int fd);
    static void cycleTask(shared_ptr<void> arg);
    static void handleTickerTimeOut(shared_ptr<void> arg);
    static void handleTimerTimeOut(shared_ptr<void> arg);
    static void handleTicker(shared_ptr<void> arg);
    static void handleTimer(shared_ptr<void> arg);
    static void readTask(shared_ptr<void> arg);
    static void writeTask(shared_ptr<void> arg);
    static void handleCloseListen(shared_ptr<void> arg);
    static void handleAddSession(shared_ptr<void> arg);
    static void handleDeleteSession(shared_ptr<void> arg);
private:
	struct EpollEventArg {
		shared_ptr<Time> t;
		shared_ptr<TcpSession> session;
		EpollEventArg(shared_ptr<Time> t, shared_ptr<TcpSession> session) : t(t), session(session) {};
	};
private:
    std::atomic<bool> needClose;
    std::atomic<bool> running;
    std::atomic<int> sessionNum;
    int readEpollFd;
	int writeEpollFd;
    int waitTime;
    int epollEventNum;
    unordered_map<int, shared_ptr<TcpSession>> sessionManager;
    unordered_map<shared_ptr<Time>, int> timeToFd;
};


#endif //NET_EPOLLTASK_H
