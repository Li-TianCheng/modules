//
// Created by ltc on 2021/5/21.
//

#include "TcpListener.h"

TcpListener::TcpListener(int port, AddressType addressType) {
    serverFd = socket(addressType, SOCK_STREAM, 0);
    bzero(&serverAddress, sizeof(serverAddress));
    if (addressType == IPV4){
        ((sockaddr_in*)(&serverAddress))->sin_family = PF_INET;
        ((sockaddr_in*)(&serverAddress))->sin_port = htons(port);
    }else {
        ((sockaddr_in6*)(&serverAddress))->sin6_family = PF_INET6;
        ((sockaddr_in6*)(&serverAddress))->sin6_port = htons(port);
        // TODO 添加IPV6
    }
    int err = bind(serverFd, &serverAddress, sizeof(serverAddress));
    if (err == -1){
        close(serverFd);
        throw std::runtime_error("listener创建失败");
    }
}

TcpListener::~TcpListener() {
    close(serverFd);
}

void TcpListener::listen() {
    int err = ::listen(serverFd, 5);
    if (err == -1){
        close(serverFd);
        throw std::runtime_error("监听失败");
    }
}

int TcpListener::accept() {
    ClientInfo* clientInfo = ObjPool::allocate<ClientInfo>();
    int clientFd = ::accept(serverFd, &clientInfo->clientAddress, &clientInfo->len);
    clientSet[clientFd] = clientInfo;
    return clientFd;
}
