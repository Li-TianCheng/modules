//
// Created by ltc on 2021/6/15.
//

#ifndef HTTP_HTTP_H
#define HTTP_HTTP_H

#include <unordered_map>
#include <string>

using std::unordered_map;
using std::string;

class Http {
public:
    explicit Http(bool isRequest);
    explicit operator string();
    bool isRequest() const;
public:
    unordered_map<string, string> line;
    unordered_map<string, string> head;
    string data;
private:
    bool isReq;
};


#endif //HTTP_HTTP_H
