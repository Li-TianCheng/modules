//
// Created by ltc on 2021/6/15.
//

#ifndef HTTP_HTTPSESSION_H
#define HTTP_HTTPSESSION_H

#include <regex>
#include <ctime>
#include "HttpServer.h"
#include "net/include/TcpSession.h"
#include "net/include/Ping.h"

struct Http;

class HttpSession : public TcpSession {
public:
    HttpSession(int bufferChunkSize);
    ~HttpSession() override = default;
    void sessionInit() override;
    void sessionClear() override;
    void handleTickerTimeOut(shared_ptr<Time> t) override;
private:
    void sendResponse(shared_ptr<Http> response);
    void handleReadDone(iter pos, size_t n) override;
    void match(shared_ptr<Http> request);
    void parse(const char& c);
    string getGMTTime();
private:
	string key;
	int count;
	int status;
	int timeout;
	shared_ptr<Time> time;
	shared_ptr<Http> request;
};

struct Http {
    unordered_map<string, string> line;
    unordered_map<string, string> head;
    vector<char> data;
};

#endif //HTTP_HTTPSESSION_H
