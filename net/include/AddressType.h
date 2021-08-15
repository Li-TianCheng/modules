//
// Created by ltc on 2021/5/24.
//

#ifndef NET_ADDRESSTYPE_H
#define NET_ADDRESSTYPE_H

#include <sys/socket.h>

enum AddressType {
    IPV4 = PF_INET,
    IPV6 = PF_INET6
};

#endif //NET_ADDRESSTYPE_H
