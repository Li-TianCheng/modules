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

enum AddressType {
    IPV4 = PF_INET,
    IPV6 = PF_INET6
};

struct ClientInfo {
    sockaddr clientAddress;
    socklen_t len;
    ClientInfo(){
        len = sizeof(clientAddress);
        bzero(&clientAddress, len);
    }
};

class TcpListener {
public:
    TcpListener(int port, AddressType addressType);
    void listen();
    int accept();
    ~TcpListener();
private:
    std::unordered_map<int, ClientInfo*> clientSet;
    sockaddr serverAddress;
    int serverFd;
};

#endif //NET_TCPLISTENER_H
