//
// Created by ltc on 2021/7/23.
//

#ifndef NET_LISTENER_H
#define NET_LISTENER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <malloc.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include "AddressType.h"
#include "TcpServerBase.h"
#include "log/include/LogSystem.h"
#include "event_system/include/EventSystem.h"
#include "AddressType.h"
#include "EpollTask.h"
#include "TcpSession.h"
#include "utils/include/StringUtils.h"

class Listener: public EventSystem {
public:
    Listener();
    void registerListener(int port, AddressType addressType, shared_ptr<TcpServerBase> server);
	bool addNewSession(shared_ptr<TcpSession> session, const string& address, AddressType addressType);
	void addListener(int port, AddressType addressType, shared_ptr<TcpServerBase> server);
    void listen();
	void closeServer(shared_ptr<TcpServerBase> server);
    ~Listener() override;
private:
    static void handleCloseListener(shared_ptr<void> arg);
    static void handleAddListener(shared_ptr<void> arg);
    static void handleAddSession(shared_ptr<void> arg);
    void cycleInit() override;
    void addNewSession(shared_ptr<TcpSession> session);
private:
	struct addNewSessionArg {
		shared_ptr<EventSystem> listener;
		shared_ptr<TcpSession> session;
	};
	struct addListenerArg {
		shared_ptr<Listener> listener;
		int port;
		AddressType addressType;
		shared_ptr<TcpServerBase> server;
	};
private:
	int count;
    int epollFd;
    int waitTime;
    int maxEpollNum;
    int epollSessionNum;
    unordered_map<int, shared_ptr<TcpServerBase>> listenMap;
    list<shared_ptr<EpollTask>> epollList;
    shared_ptr<EpollTask> waitCLose;
};


#endif //NET_LISTENER_H
