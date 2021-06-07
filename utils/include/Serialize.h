//
// Created by ltc on 2021/6/7.
//

#ifndef UTILS_SERIALIZE_H
#define UTILS_SERIALIZE_H

#include <string>
#include <jsoncpp/json/json.h>

using namespace std;

class Serialize {
public:
    template<typename T>
    static Json::Value serialize(T& obj);
    template<typename T>
    static Json::Value serialize(vector<T>& obj);
    template<typename T>
    static T deserialize(const string& str);
private:
    Serialize() = default;
};

template<typename T> inline
Json::Value Serialize::serialize(T& obj) {
    Json::Value json;
    obj.serialize(json);
    return json;
}

template<> inline
Json::Value Serialize::serialize(int& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<> inline
Json::Value Serialize::serialize(double& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<> inline
Json::Value Serialize::serialize(float& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<> inline
Json::Value Serialize::serialize(char& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<> inline
Json::Value Serialize::serialize(string& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<> inline
Json::Value Serialize::serialize(bool& obj) {
    Json::Value json;
    json["_"] = obj;
    return json;
}

template<typename T> inline
Json::Value Serialize::serialize(vector<T>& obj) {
    Json::Value json;
    for (auto& v : obj) {
        json["_"].append(serialize<T>(v));
    }
    return json;
}

template<> inline
Json::Value Serialize::serialize(vector<bool>& obj) {
    Json::Value json;
    for (auto v : obj) {
        if (v) {
            json["_"].append(true);
        } else {
            json["_"].append(false);
        }
    }
    return json;
}

template<typename T> inline
T Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    T t;
    t.deserialize(s);
    return t;
}

template<> inline
int Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    return s["_"].asInt();
}

template<> inline
double Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    return s["_"].asDouble();
}

template<> inline
float Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    return s["_"].asFloat();
}

template<> inline
char Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    return s["_"].asCString()[0];
}

template<> inline
bool Serialize::deserialize(const string& str) {
    Json::Reader reader;
    Json::Value s;
    if (!reader.parse(str, s)) {
        throw std::runtime_error("反序列化错误");
    }
    return s["_"].asBool();
}

#endif //UTILS_SERIALIZE_H
