//
// Created by ltc on 2021/6/7.
//

#include "Serialization.h"

Serialization &Serialization::getInstance() {
    static Serialization serialization;
    return serialization;
}

template<typename T>
string Serialization::serialize(T &obj) {
    return serializeElement(obj).asString();
}

template<typename T>
T Serialization::deserialize(const string &str) {
    return nullptr;
}

template<typename T, typename U, typename... V>
void Serialization::registerClass(T& _class, U& arg, V&... args) {
    getInstance().center[typeid(_class).name()].insert(varName(arg));
    registerClass(_class, args...);
}

template<typename T>
void Serialization::registerClass(T& _class) {

}

template<typename T>
Json::Value Serialization::serializeElement(T &obj) {
    Json::Value str;
    string type = typeid(obj).name();
    if (typeid(obj) == typeid(int)) {
        str["int"] = obj;
    } else if (typeid(obj) == typeid(double)) {
        str["double"] = obj;
    } else if (typeid(obj) == typeid(float)) {
        str["float"] = obj;
    } else if (typeid(obj) == typeid(bool)) {
        str["bool"] = obj;
    } else if (typeid(obj) == typeid(char)) {
        str["char"] = obj;
    } else if (typeid(obj) == typeid(string)) {
        str["string"] = obj;
    } else if (typeid(obj) == typeid(vector<int>)) {
        for (auto& v : obj) {
            str["vector<int>"].append(v);
        }
    } else if (typeid(obj) == typeid(vector<double>)) {
        for (auto& v : obj) {
            str["vector<double>"].append(v);
        }
    } else if (typeid(obj) == typeid(vector<float>)) {
        for (auto& v : obj) {
            str["vector<float>"].append(v);
        }
    } else if (typeid(obj) == typeid(vector<bool>)) {
        for (auto& v : obj) {
            str["vector<bool>"].append(v);
        }
    } else if (typeid(obj) == typeid(vector<string>)) {
        for (auto& v : obj) {
            str["vector<string>"].append(v);
        }
    } else {
        if (getInstance().center.find(typeid(obj).name()) != getInstance().center.end()) {
            unordered_set<string> fields = getInstance().center[typeid(obj).name()];
            for (auto& field : fields) {
                str[varName(field)] = serializeElement(obj.varName(field));
            }
            str[typeid(obj).name()] = str;
        }
    }
    return str;
}
