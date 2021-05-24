//
// Created by ltc on 2021/5/21.
//

#ifndef NET_TCPLISTENER_H
#define NET_TCPLISTENER_H

#include <unordered_map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <strings.h>
#include "mem_pool/include/ObjPool.hpp"
#include "task_system/include/TaskSystem.h"

enum AddressType {
    IPV4 = PF_INET,
    IPV6 = PF_INET6
};

class TcpListener;

struct ClientInfo {
    sockaddr clientAddress;
    socklen_t len;
    int clientFd;
    TcpListener* listener;
    ClientInfo(){
        len = sizeof(clientAddress);
        bzero(&clientAddress, len);
    }
};

class TcpListener {
public:
    TcpListener(int port, AddressType addressType);
    void listen();
    void close();
    ~TcpListener();
private:
    static void listenTask(void* arg);
    static void handleConnectTask(void* arg);
    void unregisterConnection(int clientFd);
    virtual void handleInput(char* buff, int recvNum) = 0;
private:
    ClientInfo* accept();
    volatile bool isClose;
    std::unordered_map<int, ClientInfo*> clientSet;
    sockaddr serverAddress;
    int serverFd;
};

#endif //NET_TCPLISTENER_H
