//
// Created by ltc on 2021/6/15.
//

#include "http/include/HttpSession.h"

void HttpSession::handleReadDone(iter pos, size_t n) {
    for (size_t i = 0; i < n; i++) {
        parse(*(pos++));
    }
    readDone(n);
}

HttpSession::HttpSession() : request(nullptr), status(0), timeout(0) {

}

void HttpSession::match(shared_ptr<Http> request) {
    LOG(Info, "HttpSession request:"+request->line["method"]+" "+request->line["url"]+" "+request->line["version"]);
    auto response = ObjPool::allocate<Http>();
    response->line["version"] = request->line["version"];
    response->line["status"] = "404";
    response->line["msg"] = "NOT FOUND";
    if (HttpServer::getMux().find(request->line["url"]) != HttpServer::getMux().end()) {
        response->line["status"] = "200";
        response->line["msg"] = "OK";
        HttpServer::getMux()[request->line["url"]](request, response);
    } else {
        for (auto& m : HttpServer::getRegexMux()) {
            if (std::regex_match(request->line["url"], std::regex(m.first))) {
                response->line["status"] = "200";
                response->line["msg"] = "OK";
                m.second(request, response);
                break;
            }
        }
    }
    response->head["Date"] = getGMTTime();
    response->head["Content-Length"] = std::to_string(response->data.size());
    if (request->head["Connection"] != "keep-alive") {
        response->head["Connection"] = "close";
    } else {
        response->head["Connection"] = "keep-alive";
        response->head["Keep-Alive"] = "timeout=30";
        timeout = 0;
    }
    sendResponse(response);
    if (response->head["Connection"] == "close") {
        closeConnection();
    }
}

void HttpSession::parse(const char &c) {
    switch(status) {
        case 0:{
            request = ObjPool::allocate<Http>();
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
                request->data.push_back(c);
            }
            break;
        }
        default: break;
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
    ping = ObjPool::allocate<Ping>(*(sockaddr_in*)&address);
    ping->send();
    uuid = addTicker(0, 0, 1, 0);
}

void HttpSession::sessionClear() {

}

void HttpSession::handleTickerTimeOut(const string &uuid) {
    if (this->uuid == uuid) {
        timeout++;
        if (!ping->recv() || timeout > 30) {
            deleteSession();
            return;
        }
        ping->send();
    }
}

void HttpSession::sendResponse(shared_ptr<Http> response) {
    string msg;
    msg += std::move(response->line["version"])+" ";
    msg += std::move(response->line["status"])+" ";
    msg += std::move(response->line["msg"])+"\r\n";
    for (auto& h : response->head) {
        msg += h.first+":"+h.second+"\r\n";
    }
    msg += "\r\n";
    auto m = ObjPool::allocate<string>(std::move(msg));
    write(m);
    auto v = ObjPool::allocate<vector<char>>(std::move(response->data));
    write(v);
}
