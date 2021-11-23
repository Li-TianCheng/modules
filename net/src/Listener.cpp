//
// Created by ltc on 2021/7/23.
//

#include "net/include/Listener.h"

Listener::Listener() : waitTime(1), waitCLose(nullptr),
                        epollSessionNum(ConfigSystem::getConfig()["system"]["net"]["listener"]["epoll_session_num"].asInt()),
                        maxEpollNum(ConfigSystem::getConfig()["system"]["net"]["listener"]["max_epoll_task_num"].asInt()) {
    signal(SIGPIPE, SIG_IGN);
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        throw std::runtime_error("epoll 申请错误");
    }
    registerEvent(EventCloseListener, handleCloseListener);
    registerEvent(EventAddSession, handleAddSession);
    registerEvent(EventAddListener, handleAddListener);
}

void Listener::registerListener(int port, AddressType addressType, shared_ptr<TcpServerBase> server) {
    server->addressType = addressType;
    server->port = port;
    server->listener = shared_from_this();
    int serverFd = socket(addressType, SOCK_STREAM|SOCK_CLOEXEC, 0);
    server->serverFd = serverFd;
    int reuse = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bzero(&server->serverAddress, sizeof(server->serverAddress));
    if (addressType == IPV4) {
        ((sockaddr_in*)(&server->serverAddress))->sin_family = PF_INET;
        ((sockaddr_in*)(&server->serverAddress))->sin_port = htons(port);
    }else {
        ((sockaddr_in6*)(&server->serverAddress))->sin6_family = PF_INET6;
        ((sockaddr_in6*)(&server->serverAddress))->sin6_port = htons(port);
        // TODO 添加IPV6
    }
    int err = bind(serverFd, &server->serverAddress, sizeof(server->serverAddress));
    if (err == -1){
        ::close(serverFd);
        throw std::runtime_error("listener创建失败");
    }
    server->epollEvent.data.fd = serverFd;
    server->epollEvent.events = Read | Err;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &server->epollEvent);
    listenMap[serverFd] = server;
}

void Listener::listen() {
    cycleInit();
    while (!listenMap.empty()) {
        cycleNoBlock(-1);
        epoll_event events[listenMap.size()];
        int num = epoll_wait(epollFd, events, listenMap.size(), waitTime);
        if (num <= 0) {
            waitTime = 1;
            addNewSession(nullptr);
        } else {
            waitTime = 0;
            for (int i = 0; i < num; i++) {
                if ((events[i].events & Err) == Err) {
                    ::close(events[i].data.fd);
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, &listenMap[events[i].data.fd]->epollEvent);
                    listenMap.erase(events[i].data.fd);
                }
                if ((events[i].events & Read) == Read) {
                    auto session = listenMap[events[i].data.fd]->getSession();
                    int clientFd = accept(events[i].data.fd, &session->address, &session->len);
                    if (clientFd > 0) {
                        session->epollEvent.data.fd = clientFd;
                        addNewSession(session);
                    }
                }
            }
        }
    }
    for (auto& e : epollList) {
        e->needClose = true;
    }
    while (true) {
        bool flag = true;
        for (auto& e : epollList) {
            if (e->isRunning()) {
                flag = false;
                break;
            }
        }
        if (flag) {
            break;
        }
        sleep(1);
    }
}

Listener::~Listener() {
    for (auto& it : listenMap) {
        ::close(it.first);
        epoll_ctl(epollFd, EPOLL_CTL_DEL, it.first, &it.second->epollEvent);
    }
    ::close(epollFd);
}

void Listener::cycleInit() {
    for (auto& it : listenMap) {
        int err = ::listen(it.first, ConfigSystem::getConfig()["system"]["net"]["listener"]["accept_num"].asInt());
        if (err == -1){
            ::close(it.first);
            throw std::runtime_error(std::to_string(it.first)+"监听失败");
        }
        LOG(Info, "port:"+std::to_string(it.second->port)+" listen begin");
    }
    auto e = ObjPool::allocate<EpollTask>(epollSessionNum);
    epollList.push_back(e);
    e->run();
}

void Listener::handleCloseListener(shared_ptr<void> arg) {
	auto server = static_pointer_cast<TcpServerBase>(arg);
	auto listener = static_pointer_cast<Listener>(server->listener.lock());
	if (listener != nullptr) {
		epoll_ctl(listener->epollFd, EPOLL_CTL_DEL, server->serverFd, &server->epollEvent);
		::shutdown(server->serverFd, SHUT_RD);
		::close(server->serverFd);
		listener->listenMap.erase(server->serverFd);
		LOG(Info, "port:"+std::to_string(server->port)+" listen end");
	}
}

void Listener::addNewSession(shared_ptr<TcpSession> session) {
    if (session == nullptr && waitCLose != nullptr && waitCLose->sessionNum == 0) {
        malloc_trim(0);
        waitCLose->needClose = true;
        epollList.remove(waitCLose);
        waitCLose = nullptr;
        std::ostringstream log;
        log << "EpollList decrease, current num:" << epollList.size();
        LOG(Info, log.str());
    }
    int min = INT32_MAX;
    shared_ptr<EpollTask> minIter = nullptr;
    int sum = 0;
    vector<int> snapshot;
    for (auto& e : epollList) {
        int num = e->sessionNum;
        snapshot.push_back(num);
        sum += num;
        if (num <= min) {
            min = num;
            minIter = e;
        }
    }
    int second = INT32_MAX;
    shared_ptr<EpollTask> tarIter = nullptr;
    int count = -1;
    for (auto& e : epollList) {
        count++;
        if (e == minIter) {
            continue;
        }
        int num = snapshot[count];
        if (num <= second) {
            second = num;
            tarIter = e;
        }
    }
    if (waitCLose != nullptr && epollList.size() > 1) {
        waitCLose = minIter;
        if (session != nullptr) {
            session->epoll = tarIter;
            auto e = ObjPool::allocate<Event>(EventAddSession, session);
            tarIter->receiveEvent(e);
            tarIter->sessionNum++;
        }
        if (second+1 >= epollSessionNum && epollList.size() < maxEpollNum) {
            waitCLose = nullptr;
        }
    } else {
        if (session != nullptr) {
            session->epoll = minIter;
            auto e = ObjPool::allocate<Event>(EventAddSession, session);
            minIter->receiveEvent(e);
            minIter->sessionNum++;
        }
        if (min+1 >= epollSessionNum && epollList.size() < maxEpollNum) {
            auto e = ObjPool::allocate<EpollTask>(epollSessionNum);
            epollList.push_back(e);
            std::ostringstream log;
            log << "EpollList increase, current num:" << epollList.size();
            LOG(Info, log.str());
            e->run();
        } else if (epollList.size() > 1 && sum / epollList.size() < epollSessionNum/maxEpollNum) {
            waitCLose = minIter;
        }
    }
}

void Listener::handleAddListener(shared_ptr<void> arg) {
    auto _arg = static_pointer_cast<addListenerArg>(arg);
    _arg->listener->registerListener(_arg->port, _arg->addressType, _arg->server);
    int err = ::listen(_arg->server->serverFd, ConfigSystem::getConfig()["system"]["net"]["listener"]["accept_num"].asInt());
    if (err == -1){
        ::close(_arg->server->serverFd);
        return;
    }
    LOG(Info, "port:"+std::to_string(_arg->port)+" listen begin");
}

void Listener::handleAddSession(shared_ptr<void> arg) {
    auto _arg = static_pointer_cast<addNewSessionArg>(arg);
    auto session = _arg->session;
    vector<string> split = utils::split(_arg->address, ':');
    session->epollEvent.data.fd = socket(_arg->addressType, SOCK_STREAM, 0);
    bzero(&session->address, sizeof(session->address));
    if (_arg->addressType == IPV4) {
        ((sockaddr_in*)(&session->address))->sin_family = PF_INET;
        if ((inet_addr(split[0].data())) == INADDR_NONE) {
            hostent* host = gethostbyname(split[0].data());
            if (host == nullptr) {
                return;
            }
            ((sockaddr_in*)(&session->address))->sin_addr = *(in_addr*)host->h_addr;
        } else {
            inet_pton(PF_INET, split[0].data(), &((sockaddr_in*)(&session->address))->sin_addr);
        }
        ((sockaddr_in*)(&session->address))->sin_port = htons(std::stoi(split[1]));
    } else {
        ((sockaddr_in6*)(&session->address))->sin6_family = PF_INET6;
        inet_pton(PF_INET6, split[0].data(), &((sockaddr_in6*)(&session->address))->sin6_addr);
        ((sockaddr_in6*)(&session->address))->sin6_port = htons(std::stoi(split[1]));
        // TODO IPV6
    }
    int err = ::connect(session->epollEvent.data.fd, &session->address, sizeof(session->address));
    if (err == -1) {
        return;
    }
    _arg->listener->addNewSession(session);
}
