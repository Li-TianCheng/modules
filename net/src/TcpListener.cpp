//
// Created by ltc on 2021/5/21.
//

#include "TcpListener.h"

TcpListener::TcpListener(int port, AddressType addressType): isClose(false) {
    serverFd = socket(addressType, SOCK_STREAM, 0);
    bzero(&serverAddress, sizeof(serverAddress));
    if (addressType == IPV4) {
        ((sockaddr_in*)(&serverAddress))->sin_family = PF_INET;
        ((sockaddr_in*)(&serverAddress))->sin_port = htons(port);
    }else {
        ((sockaddr_in6*)(&serverAddress))->sin6_family = PF_INET6;
        ((sockaddr_in6*)(&serverAddress))->sin6_port = htons(port);
        // TODO 添加IPV6
    }
    int err = bind(serverFd, &serverAddress, sizeof(serverAddress));
    if (err == -1){
        ::close(serverFd);
        throw std::runtime_error("listener创建失败");
    }
}

TcpListener::~TcpListener() {
    if (!isClose){
        ::close(serverFd);
    }
    while (!clientSet.empty()){}
}

void TcpListener::listen() {
    int err = ::listen(serverFd, 5);
    if (err == -1){
        ::close(serverFd);
        throw std::runtime_error("监听失败");
    }
    TaskSystem::addTask(listenTask, this);
}

ClientInfo* TcpListener::accept() {
    ClientInfo* clientInfo = ObjPool::allocate<ClientInfo>();
    int clientFd = ::accept(serverFd, &clientInfo->clientAddress, &clientInfo->len);
    clientInfo->clientFd = clientFd;
    clientInfo->listener = this;
    if (clientFd != -1){
        clientSet[clientFd] = clientInfo;
    }
    return clientInfo;
}

void TcpListener::unregisterConnection(int clientFd) {
    if (clientSet.find(clientFd) != clientSet.end()) {
        ::close(clientFd);
        ClientInfo* clientInfo = clientSet[clientFd];
        clientSet.erase(clientFd);
        ObjPool::deallocate(clientInfo);
    }
}

void TcpListener::listenTask(void *arg) {
    while (true) {
        ClientInfo* clientInfo = ((TcpListener*)arg)->accept();
        if (clientInfo->clientFd == -1){
            ObjPool::deallocate(clientInfo);
            if (clientInfo->listener->isClose){
                break;
            }
        } else{
            TaskSystem::addTask(handleConnectTask, clientInfo);
        }
    }
}

void TcpListener::handleConnectTask(void *arg) {
    ClientInfo* clientInfo = (ClientInfo*)arg;
    clientInfo->listener->handleDoTask(clientInfo);
    clientInfo->listener->unregisterConnection(clientInfo->clientFd);
}

void TcpListener::close() {
    if (!isClose){
        isClose = true;
        ::shutdown(serverFd, SHUT_RD) ;
        ::close(serverFd);
    }
}
