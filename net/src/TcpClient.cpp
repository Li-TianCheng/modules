//
// Created by ltc on 2021/5/24.
//

#include "TcpClient.h"

TcpClient::TcpClient(const string &address, AddressType addressType) {
    vector<string> split = utils::split(address, ':');
    clientFd = socket(addressType, SOCK_STREAM, 0);
    bzero(&serverAddress, sizeof(serverAddress));
    if (addressType == IPV4) {
        ((sockaddr_in*)(&serverAddress))->sin_family = PF_INET;
        inet_pton(PF_INET, split[0].data(), &((sockaddr_in*)(&serverAddress))->sin_addr);
        ((sockaddr_in*)(&serverAddress))->sin_port = htons(std::stoi(split[1]));
    } else {
        ((sockaddr_in6*)(&serverAddress))->sin6_family = PF_INET6;
        inet_pton(PF_INET6, split[0].data(), &((sockaddr_in6*)(&serverAddress))->sin6_addr);
        ((sockaddr_in6*)(&serverAddress))->sin6_port = htons(std::stoi(split[1]));
        // TODO IPV6
    }
}

void TcpClient::connect() {
    int err = ::connect(clientFd, &serverAddress, sizeof(serverAddress));
    if (err == -1) {
        throw std::runtime_error("发起连接错误");
    }
}

void TcpClient::write(const string &context, int length) {
    int idx = 0;
    while (idx < context.length()) {
        send(clientFd, context.substr(idx, length).data(), context.size(), 0);
        idx += length;
    }
}

string TcpClient::read() {
    int recvNum = recv(clientFd, readBuff, sizeof(readBuff), 0);
    string context = readBuff;
    context.resize(recvNum);
    return context;
}

void TcpClient::close() {
    ::close(clientFd);
}
