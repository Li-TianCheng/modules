//
// Created by ltc on 2021/6/16.
//

#ifndef HTTP_HTTPSERVER_H
#define HTTP_HTTPSERVER_H

#include "net/include/Server.h"
#include "net/include/Listener.h"
#include "HttpSession.h"

class HttpSession;
struct Http;

class HttpServer {
public:
    HttpServer(int port, AddressType addressType);
    HttpServer(shared_ptr<Listener> listener, int port, AddressType addressType);
    void serve();
    void close();
    void registerHandler(const string& pattern, void(*handle)(shared_ptr<Http>, shared_ptr<Http>));
    void registerRegexHandler(const string& pattern, void(*handle)(shared_ptr<Http>, shared_ptr<Http>));
    void unregisterHandler(const string& pattern);
    void unregisterRegexHandler(const string& pattern);
private:
    friend class HttpSession;
    static unordered_map<string, void(*)(shared_ptr<Http>, shared_ptr<Http>)>& getRegexMux();
    static unordered_map<string, void(*)(shared_ptr<Http>, shared_ptr<Http>)>& getMux();
private:
    shared_ptr<Server<HttpSession>> server;
    shared_ptr<Listener> listener;
};


#endif //HTTP_HTTPSERVER_H
