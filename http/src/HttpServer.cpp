//
// Created by ltc on 2021/6/16.
//

#include "http/include/HttpServer.h"

HttpServer::HttpServer(int port, AddressType addressType) : server(port, addressType) {

}

void HttpServer::serve() {
    server.cycle();
}

void HttpServer::close() {
    server.close();
}

void HttpServer::registerHandler(const string &pattern, void (*handle)(const shared_ptr<Http>&, const shared_ptr<Http>&)) {
    getMux()[pattern] = handle;
}

void HttpServer::registerRegexHandler(const string &pattern, void (*handle)(const shared_ptr<Http>&, const shared_ptr<Http>&)) {
    getRegexMux()[pattern] = handle;
}

void HttpServer::unregisterHandler(const string &pattern) {
    if (getMux().find(pattern) != getMux().end()) {
        getMux().erase(pattern);
    }
}

void HttpServer::unregisterRegexHandler(const string &pattern) {
    if (getRegexMux().find(pattern) != getRegexMux().end()) {
        getRegexMux().erase(pattern);
    }
}

unordered_map<string, void (*)(const shared_ptr<Http>&, const shared_ptr<Http>&)> &HttpServer::getRegexMux() {
    static unordered_map<string, void(*)(const shared_ptr<Http>&, const shared_ptr<Http>&)> regexMux;
    return regexMux;
}

unordered_map<string, void (*)(const shared_ptr<Http>&, const shared_ptr<Http>&)> &HttpServer::getMux() {
    static unordered_map<string, void(*)(const shared_ptr<Http>&, const shared_ptr<Http>&)> mux;
    return mux;
}
