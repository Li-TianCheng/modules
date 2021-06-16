//
// Created by ltc on 2021/6/15.
//

#include "http/include/Http.h"

Http::Http(bool isRequest) : isReq(isRequest) {

}

Http::operator string() {
    string str;
    if (isReq) {
        str += line["method"] + " ";
        str += line["url"] + " ";
        str += line["version"] + "\r\n";
    } else {
        str += line["version"] + " ";
        str += line["status"] + " ";
        str += line["msg"] + "\r\n";
    }
    for (auto& k : head) {
        str += k.first + ":" + k.second + "\r\n";
    }
    str += "\r\n" + data;
    return str;
}

bool Http::isRequest() const {
    return isReq;
}
