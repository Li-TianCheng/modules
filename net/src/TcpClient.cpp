//
// Created by ltc on 2021/5/24.
//

#include "TcpClient.h"
#include <iostream>

TcpClient::TcpClient(const string &address, AddressType addressType) {
    vector<string> split = utils::split(address, ':');
    clientFd = socket(addressType, SOCK_STREAM, 0);
    bzero(&serverAddress, sizeof(serverAddress));
    if (addressType == IPV4) {
        ((sockaddr_in*)(&serverAddress))->sin_family = PF_INET;
        if ((inet_addr(split[0].data())) == INADDR_NONE) {
            hostent* host = gethostbyname(split[0].data());
            if (host == nullptr) {
                throw std::runtime_error("host错误");
            }
            ((sockaddr_in*)(&serverAddress))->sin_addr = *(in_addr*)host->h_addr;
        } else {
            inet_pton(PF_INET, split[0].data(), &((sockaddr_in*)(&serverAddress))->sin_addr);
        }
        ((sockaddr_in*)(&serverAddress))->sin_port = htons(std::stoi(split[1]));
    } else {
        ((sockaddr_in6*)(&serverAddress))->sin6_family = PF_INET6;
        inet_pton(PF_INET6, split[0].data(), &((sockaddr_in6*)(&serverAddress))->sin6_addr);
        ((sockaddr_in6*)(&serverAddress))->sin6_port = htons(std::stoi(split[1]));
        // TODO IPV6
    }
}

bool TcpClient::connect() {
    int err = ::connect(clientFd, &serverAddress, sizeof(serverAddress));
    if (err == -1) {
        return false;
    }
    return true;
}

void TcpClient::write(const char* c, int n) {
    send(clientFd, c, n, 0);
}

string TcpClient::read() {
    string msg;
    int recvNum = recv(clientFd, buffer, sizeof(buffer), 0);
    msg = buffer;
    msg.resize(recvNum);
    return msg;
}

void TcpClient::close() {
    ::close(clientFd);
}
