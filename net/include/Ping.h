//
// Created by ltc on 2021/6/22.
//

#ifndef NET_PING_H
#define NET_PING_H

#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <netinet/ip_icmp.h>
#include <zconf.h>
#include <sys/time.h>
#include <netdb.h>
#include "AddressType.h"
#include "utils/include/StringUtils.h"

using std::string;
using std::vector;

class Ping {
public:
    explicit Ping(sockaddr_in& address);
    explicit Ping(const string& address);
    bool send();
    bool recv();
    ~Ping();
private:
    unsigned short check(char* pIcmp);
private:
    int fd;
    sockaddr_in address;
    char writeBuff[64];
    char readBuff[128];
};


#endif //NET_PING_H
