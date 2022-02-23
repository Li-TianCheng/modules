//
// Created by ltc on 2021/5/28.
//

#ifndef NET_SERVER_H
#define NET_SERVER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <csignal>
#include <malloc.h>
#include "AddressType.h"
#include "Session.h"
#include "ServerBase.h"

template<typename T>
class Server: public ServerBase {
public:
    Server();
    std::shared_ptr<Session> getSession() override;
    ~Server() override = default;
private:
    int bufferChunkSize;
};

template<typename T>
std::shared_ptr<Session> Server<T>::getSession() {
    auto session = ObjPool::allocate<T>(bufferChunkSize);
    session->server = shared_from_this();
    return session;
}

template<typename T>
Server<T>::Server(): bufferChunkSize(ConfigSystem::getConfig()["system"]["net"]["read_buffer_chunk_size"].asInt()) {

}


#endif //NET_SERVER_H
