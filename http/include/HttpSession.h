//
// Created by ltc on 2021/6/15.
//

#ifndef HTTP_HTTPSESSION_H
#define HTTP_HTTPSESSION_H

#include <regex>
#include <ctime>
#include "http/include/Http.h"
#include "HttpServer.h"
#include "net/include/TcpSession.h"
#include "net/include/Ping.h"


class HttpSession : public TcpSession {
public:
    HttpSession();
    ~HttpSession() override = default;
    void sessionInit() override;
    void sessionClear() override;
    void handleTickerTimeOut(const string &uuid) override;
private:
    void handleReadDone(const string& recvMsg) override;
    void match(const shared_ptr<Http>& request);
    void parse(char& c);
    string getGMTTime();
private:
    Ping ping;
    bool isFirstPing;
    shared_ptr<Http> request;
    string key;
    int count;
    int status;
    int timeout;
    string uuid;
};


#endif //HTTP_HTTPSESSION_H
