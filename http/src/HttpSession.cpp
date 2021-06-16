//
// Created by ltc on 2021/6/15.
//

#include "http/include/HttpSession.h"

void HttpSession::handleReadDone(const string &recvMsg) {
    for (auto c : recvMsg) {
        parse(c);
    }

}

HttpSession::HttpSession() : request(nullptr), status(0), timeout(0) {

}

void HttpSession::match(Http* request) {
    Http response(false);
    response.line["version"] = request->line["version"];
    response.line["status"] = "404";
    response.line["msg"] = "NOT FOUND";
    if (HttpServer::getMux().find(request->line["url"]) != HttpServer::getMux().end()) {
        response.line["status"] = "200";
        response.line["msg"] = "OK";
        HttpServer::getMux()[request->line["url"]](request, &response);
    } else {
        for (auto& m : HttpServer::getRegexMux()) {
            if (std::regex_match(request->head["url"], std::regex(m.first))) {
                response.line["status"] = "200";
                response.line["msg"] = "OK";
                m.second(request, &response);
                break;
            }
        }
    }
    response.head["Date"] = getGMTTime();
    response.head["Content-Length"] = std::to_string(response.data.size());
    if (request->head["Connection"] != "keep-alive") {
        response.head["Connection"] = "close";
    } else {
        response.head["Connection"] = "keep-alive";
        timeout = 0;
    }
    write(string(response));
    if (response.head["Connection"] == "close") {
        closeConnection();
    }
    ObjPool::deallocate(request);
}

void HttpSession::parse(char &c) {
    switch(status) {
        case 0:{
            request = ObjPool::allocate<Http>(true);
            status = 1;
        }
        case 1:{
            if (c == ' ') {
                status = 2;
            } else {
                request->line["method"] += c;
            }
            break;
        }
        case 2:{
            if (c == ' ') {
                status = 3;
            } else {
                request->line["url"] += c;
            }
            break;
        }
        case 3:{
            if (c == '\r') {
                status = 4;
            } else {
                request->line["version"] += c;
            }
            break;
        }
        case 4:{
            if (c == '\n') {
                key = "";
                status = 5;
            } else {
                ObjPool::deallocate(request);
                request = nullptr;
                status = 0;
            }
            break;
        }
        case 5:{
            if (c == ':') {
                status = 6;
            } else if (c == '\r') {
                status = 7;
            } else {
                key += c;
            }
            break;
        }
        case 6:{
            if (c == '\r') {
                request->head[key].erase(0, request->head[key].find_first_not_of(' '));
                request->head[key].erase(request->head[key].find_last_not_of(' ')+1);
                status = 4;
            } else {
                request->head[key] += c;
            }
            break;
        }
        case 7:{
            if (c == '\n') {
                if (request->head.find("Content-Length") == request->head.end() || request->head["Content-Length"] == "0") {
                    match(request);
                    request = nullptr;
                    status = 0;
                } else {
                    count = 0;
                    status = 8;
                }
            } else {
                ObjPool::deallocate(request);
                request = nullptr;
                status = 0;
            }
            break;
        }
        case 8:{
            count++;
            if (request->head["Content-Length"] ==  std::to_string(count)) {
                match(request);
                request = nullptr;
                status = 0;
            } else {
                request->data += c;
            }
            break;
        }
        default: break;
    }
}

HttpSession::~HttpSession() {
    if (request != nullptr) {
        ObjPool::deallocate(request);
    }
}

string HttpSession::getGMTTime() {
    time_t now = time(nullptr);
    tm* gmt = gmtime(&now);
    const char* fmt = "%a, %d %b %Y %H:%M:%S GMT";
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), fmt, gmt);
    return timeStr;
}

void HttpSession::sessionInit() {
    uuid = addTicker(0, 0, 1, 0);
}

void HttpSession::sessionClear() {
    deleteTicker(uuid);
}

void HttpSession::handleTickerTimeOut(const string &uuid) {
    if (this->uuid == uuid) {
        timeout++;
        if (timeout == 30) {
            closeConnection();
        }
    }
}
