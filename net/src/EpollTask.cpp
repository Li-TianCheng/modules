//
// Created by ltc on 2021/7/23.
//

#include "EpollTask.h"

EpollTask::EpollTask() : running(false), needClose(false), sessionNum(0), waitTime(WaitTime) {
    epollFd = epoll_create(1);
    if (epollFd == -1) {
        throw std::runtime_error("epoll 申请错误");
    }
    init();
    std::ostringstream log;
    log << "EpollTask[" << this << "] begin";
    LOG(Info, log.str());
}

EpollTask::~EpollTask() {
    while (running) {
        needClose = true;
    }
    ::close(epollFd);
}

void EpollTask::run() {
    TaskSystem::addTask(cycleTask, shared_from_this());
}

void EpollTask::init() {
    registerEvent(EventEndCycle, nullptr);
    registerEvent(EventTickerTimeOut, handleTickerTimeOut);
    registerEvent(EventTimerTimeOut, handleTimerTimeOut);
    registerEvent(EventCloseListen, handleCloseListen);
    registerEvent(EventCloseConnection, handleCloseConnection);
    registerEvent(EventTicker, handleTicker);
    registerEvent(EventTimer, handleTimer);
    registerEvent(EventAddSession, handleAddSession);
}

void EpollTask::handleAddSession(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    static_pointer_cast<EpollTask>(session->epoll)->addNewSession(session);
}

void EpollTask::addNewSession(shared_ptr<TcpSession> session) {
    int fd = session->epollEvent.data.fd;
    int flags = fcntl(fd, 0);
    fcntl(fd, F_SETFL, flags|O_NONBLOCK);
    session->epollFd = epollFd;
    session->epollEvent.events = Read|Err|Hup|RdHup|Et|OneShot;
    session->sessionInit();
    sessionManager[fd] = session;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &session->epollEvent);
    std::ostringstream log;
    log << "EpollTask[" << this << "] addNewSession[" << session << "]";
    LOG(Info, log.str());
}

void EpollTask::delSession(int fd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &sessionManager[fd]->epollEvent);
    if (sessionManager.find(fd) != sessionManager.end()) {
        auto session = sessionManager[fd];
        session->sessionClear();
        sessionManager.erase(fd);
        sessionNum--;
        std::ostringstream log;
        log << "EpollTask[" << this << "] deleteSession[" << session << "]";
        LOG(Info, log.str());
    }
    ::close(fd);
}

void EpollTask::handleTickerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<Time>(arg);
    auto e = static_pointer_cast<EpollTask>(t->ePtr.lock());
    if (e == nullptr) {
        TimeSystem::deleteTicker(t);
        return;
    }
    if (e->timeToFd.find(t) != e->timeToFd.end()) {
        int fd = e->timeToFd[t];
        if (e->sessionManager.find(fd) != e->sessionManager.end()) {
            e->sessionManager[fd]->handleTimerTimeOut(t->uuid);
        } else {
            e->timeToFd.erase(t);
            TimeSystem::deleteTicker(t);
        }
    }
}

void EpollTask::handleTimerTimeOut(shared_ptr<void> arg) {
    auto t = static_pointer_cast<Time>(arg);
    auto e = static_pointer_cast<EpollTask>(t->ePtr.lock());
    if (e == nullptr) {
        TimeSystem::deleteTicker(t);
        return;
    }
    if (e->timeToFd.find(t) != e->timeToFd.end()) {
        int fd = e->timeToFd[t];
        if (e->sessionManager.find(fd) != e->sessionManager.end()) {
            e->sessionManager[fd]->handleTimerTimeOut(t->uuid);
        }
        e->timeToFd.erase(t);
    }
}

void EpollTask::handleTicker(shared_ptr<void> arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    auto e = static_pointer_cast<EpollTask>(t->ePtr.lock());
    if (e == nullptr) {
        TimeSystem::deleteTicker(t);
        return;
    }
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->timeToFd[t] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTicker, t);
    }
}

void EpollTask::handleTimer(shared_ptr<void> arg) {
    auto session = static_pointer_cast<EpollEventArg>(arg)->session;
    auto t = static_pointer_cast<EpollEventArg>(arg)->t;
    auto e = static_pointer_cast<EpollTask>(t->ePtr.lock());
    if (e == nullptr) {
        TimeSystem::deleteTicker(t);
        return;
    }
    if (e->sessionManager.find(session->epollEvent.data.fd) != e->sessionManager.end()) {
        e->timeToFd[t] = session->epollEvent.data.fd;
        TimeSystem::receiveEvent(EventTimer, t);
    }
}

bool EpollTask::isRunning() {
    return running;
}

void EpollTask::readTask(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    std::ostringstream log;
    log << "session[" << session << "] readTask start";
    LOG(Info, log.str());
    int res = session->readBuffer.readFromFd(session->epollEvent.data.fd);
    if (res == -1) {
        session->epollEvent.events = Read|Err|Hup|RdHup|Et|OneShot;
        session->isRead = false;
        epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
        std::ostringstream log;
        log << "session[" << session << "] readTask failed";
        LOG(Warn, log.str());
        return;
    }
    session->handleReadDone(session->readBuffer.getReadPos(), session->readBuffer.getMsgNum());
    session->isRead = false;
    epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
    std::ostringstream _log;
    _log << "session[" << session << "] readTask done";
    LOG(Info, _log.str());
}

void EpollTask::writeTask(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    std::ostringstream log;
    log << "session[" << session << "] writeTask start";
    LOG(Info, log.str());
    deque<Msg> temp;
    session->mutex.lock();
    while (!session->msgQueue.empty()) {
        temp.push_back(std::move(session->msgQueue.front()));
        session->msgQueue.pop_front();
    }
    session->mutex.unlock();
    while (!temp.empty()) {
        while (true) {
            ssize_t sendNum;
            if (temp.front().type == 0) {
                if (temp.front().offset == static_pointer_cast<string>(temp.front().msg)->size()) {
                    break;
                }
                sendNum = send(session->epollEvent.data.fd, static_pointer_cast<string>(temp.front().msg)->data()+temp.front().offset, static_pointer_cast<string>(temp.front().msg)->size()-temp.front().offset, MSG_DONTWAIT);
            }
            if (temp.front().type == 1) {
                if (temp.front().offset == static_pointer_cast<vector<char>>(temp.front().msg)->size()) {
                    break;
                }
                sendNum = send(session->epollEvent.data.fd, static_pointer_cast<vector<char>>(temp.front().msg)->data()+temp.front().offset, static_pointer_cast<vector<char>>(temp.front().msg)->size()-temp.front().offset, MSG_DONTWAIT);
            }
            if (sendNum <= 0) {
                if (errno == EAGAIN) {
                    session->mutex.lock();
                    while (!temp.empty()) {
                        session->msgQueue.push_front(std::move(temp.back()));
                        temp.pop_back();
                    }
                    session->mutex.unlock();
                    session->isWrite = false;
                    epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
                    std::ostringstream log;
                    log << "session[" << session << "] writeTask eagain";
                    LOG(Info, log.str());
                    return;
                } else {
                    session->epollEvent.events = Read|Err|Hup|RdHup|Et|OneShot;
                    session->isWrite = false;
                    epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
                    std::ostringstream log;
                    log << "session[" << session << "] writeTask failed";
                    LOG(Warn, log.str());
                    return;
                }
            }
            temp.front().offset += sendNum;
        }
        temp.pop_front();
    }
    session->mutex.lock();
    if (session->msgQueue.empty()) {
        session->epollEvent.events &= ~Write;
    }
    session->mutex.unlock();
    if (session->isCloseConnection) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
    }
    session->isWrite = false;
    epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
    std::ostringstream _log;
    _log << "session[" << session << "] writeTask done";
    LOG(Info, _log.str());
}

void EpollTask::cycleTask(shared_ptr<void> arg) {
    auto epoll = static_pointer_cast<EpollTask>(arg);
    epoll->running = true;
    epoll->cycleInit();
    while (!epoll->needClose || epoll->sessionNum != 0 || !epoll->timeToFd.empty()) {
        epoll->cycleNoBlock(-1);
        epoll_event events[EpollEventNum];
        int num = epoll_wait(epoll->epollFd, events, EpollEventNum, epoll->waitTime);
        if (num == 0) {
            epoll->waitTime = WaitTime;
        } else {
            epoll->waitTime = 0;
        }
        for (int i = 0; i < num; i++) {
            auto event = events[i];
            if (((event.events & RdHup) == RdHup) || ((event.events & Err) == Err) || ((event.events & Hup) == Hup)) {
                epoll->delSession(event.data.fd);
            } else {
                if ((event.events & Read) == Read) {
                    auto session = epoll->sessionManager[event.data.fd];
                    if (!session->isRead) {
                        session->isRead = true;
                        TaskSystem::addTask(readTask, session);
                    }
                }
                if ((event.events & Write) == Write) {
                    auto session = epoll->sessionManager[event.data.fd];
                    if (!session->isWrite) {
                        session->isWrite = true;
                        TaskSystem::addPriorityTask(writeTask, session);
                    }
                }
            }
        }
    }
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    epoll->receiveEvent(e);
    epoll->running = false;
    std::ostringstream log;
    log << "EpollTask[" << epoll << "] close";
    LOG(Info, log.str());
}

void EpollTask::handleCloseConnection(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    session->mutex.lock();
    if (session->msgQueue.empty()) {
        ::shutdown(session->epollEvent.data.fd, SHUT_RD);
        epoll_ctl(session->epollFd, EPOLL_CTL_MOD, session->epollEvent.data.fd, &session->epollEvent);
    }
    session->mutex.unlock();
}

void EpollTask::handleCloseListen(shared_ptr<void> arg) {
    auto session = static_pointer_cast<TcpSession>(arg);
    session->server->close();
}
