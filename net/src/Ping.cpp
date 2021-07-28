//
// Created by ltc on 2021/6/22.
//

#include "Ping.h"
#include <iostream>

Ping::Ping(const sockaddr_in &address) : address(address) {
    fd = socket(IPV4, SOCK_RAW, IPPROTO_ICMP);
}

Ping::Ping(const string &address) {
    fd = socket(IPV4, SOCK_RAW, IPPROTO_ICMP);
    bzero(&this->address, sizeof(this->address));
    this->address.sin_family = PF_INET;
    if ((inet_addr(address.data())) == INADDR_NONE) {
        hostent* host = gethostbyname(address.data());
        if (host == nullptr) {
            throw std::runtime_error("host错误");
        }
        this->address.sin_addr = *(in_addr*)host->h_addr;
    } else {
        inet_pton(PF_INET, address.data(), &((sockaddr_in*)(&this->address))->sin_addr);
    }
}

unsigned short Ping::check(char* pIcmp) {
    int len = 64;
    int sum = 0;
    unsigned short* p = (unsigned short*)pIcmp;
    unsigned short answer = 0;
    while (len > 1) {
        sum += *p++;
        len -= 2;
    }
    if (len == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)p;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

bool Ping::recv() {
    if (strcmp(inet_ntoa(address.sin_addr), "127.0.0.1") == 0){
        return true;
    }
    sockaddr_in remote;
    socklen_t len = sizeof(remote);
    int recvNum = recvfrom(fd, readBuff, sizeof(readBuff), MSG_DONTWAIT, (sockaddr*)&remote, &len);
    if (recvNum <= 0) {
        return false;
    }
    int ipLen = (((ip*)readBuff)->ip_hl << 2);
    if (recvNum - ipLen < 8) {
        return false;
    }
    icmp* p = (icmp*)(readBuff + ipLen);
    if (p->icmp_type == ICMP_ECHOREPLY && p->icmp_id == getpid() && inet_ntoa(address.sin_addr) == inet_ntoa(remote.sin_addr)) {
        return true;
    }
    return false;
}

bool Ping::send() {
    if (strcmp(inet_ntoa(address.sin_addr), "127.0.0.1") == 0){
        return true;
    }
    icmp* p = (icmp*)writeBuff;
    p->icmp_type = ICMP_ECHO;
    p->icmp_code = 0;
    p->icmp_seq = 0;
    p->icmp_id = getpid();
    p->icmp_cksum = 0;
    timeval* time = (timeval*)p->icmp_data;
    gettimeofday(time, nullptr);
    p->icmp_cksum = check(writeBuff);
    if ((::sendto(fd, writeBuff, sizeof(writeBuff), 0, (sockaddr*)&address, sizeof(address))) < 0) {
        return false;
    }
    return true;
}

Ping::~Ping() {
    close(fd);
}

