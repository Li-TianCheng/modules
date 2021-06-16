//
// Created by ltc on 2021/6/16.
//

#ifndef HTTP_HTTPSERVER_H
#define HTTP_HTTPSERVER_H

#include "net/include/TcpServer.h"
#include "HttpSession.h"
#include "Http.h"

class HttpSession;

class HttpServer {
public:
    HttpServer(int port, AddressType addressType);
    void serve();
    void close();
    void registerHandler(const string& pattern, void(*handle)(Http*, Http*));
    void registerRegexHandler(const string& pattern, void(*handle)(Http*, Http*));
    void unregisterHandler(const string& pattern);
    void unregisterRegexHandler(const string& pattern);
private:
    friend class HttpSession;
    static unordered_map<string, void(*)(Http*, Http*)>& getRegexMux();
    static unordered_map<string, void(*)(Http*, Http*)>& getMux();
private:
    TcpServer<HttpSession> server;
};


#endif //HTTP_HTTPSERVER_H
