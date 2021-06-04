//
// Created by ltc on 2021/5/28.
//

#ifndef NET_EPOLLEVENTTYPE_H
#define NET_EPOLLEVENTTYPE_H

#include <sys/epoll.h>

enum EpollEventType {
    Read = EPOLLIN,
    Write = EPOLLOUT,
    Err = EPOLLERR,
    Et = EPOLLET,
    RdHup = EPOLLRDHUP,
    OneShot = EPOLLONESHOT
};

#endif //NET_EPOLLEVENTTYPE_H
