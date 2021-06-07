//
// Created by ltc on 2021/6/7.
//

#ifndef UTILS_SERIALIZATION_H
#define UTILS_SERIALIZATION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <jsoncpp/json/json.h>

#define varName(x) x

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::tuple;
using std::get;

class Serialization {
public:
    template<typename T>
    void registerClass(T& _class);
    template<typename T, typename U, typename... V>
    void registerClass(T& _class, U& arg, V&... args);
    template<typename T>
    string serialize(T& obj);
    template<typename T>
    T deserialize(const string& str);
private:
    template<typename T>
    Json::Value serializeElement(T& obj);
    Serialization() = default;
    static Serialization& getInstance();
private:
    unordered_map<string, unordered_set<string>> center;
};


#endif //UTILS_SERIALIZATION_H
