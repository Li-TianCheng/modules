//
// Created by ltc on 2021/5/24.
//

#ifndef NET_TCPCLIENT_H
#define NET_TCPCLIENT_H

#include <cstring>
#include <vector>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include "utils/include/StringUtils.h"
#include "BufferIo.h"
#include "AddressType.h"

using std::string;
using std::vector;

class TcpClient {
public:
    TcpClient(const string& address, AddressType addressType);
    void connect();
    void write(const string& context);
    void close();
    string read();
private:
    BufferIo bufferIo;
    int clientFd;
    sockaddr serverAddress;
};


#endif //NET_TCPCLIENT_H
