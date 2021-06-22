//
// Created by ltc on 2021/5/24.
//

#ifndef NET_TCPCLIENT_H
#define NET_TCPCLIENT_H

#include <cstring>
#include <vector>
#include <netinet/in.h>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <unistd.h>
#include "utils/include/StringUtils.h"
#include "AddressType.h"

using std::string;
using std::vector;

static const int ReadBufferSize = 256;

class TcpClient {
public:
    TcpClient(const string& address, AddressType addressType);
    void connect();
    void write(const string& context);
    void close();
    string read();
private:
    int clientFd;
    sockaddr serverAddress;
    char buffer[ReadBufferSize];
};


#endif //NET_TCPCLIENT_H
