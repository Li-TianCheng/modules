//
// Created by ltc on 2021/6/7.
//

#ifndef UTILS_SERIALIZE_H
#define UTILS_SERIALIZE_H

#include <string>
#include <vector>

using namespace std;

template<typename T>
vector<char> serialize(T& obj) {
    return vector<char>((char*)&obj, (char*)&obj+sizeof(T));
}

template<typename T>
vector<char> serialize(vector<T>& obj) {
    return vector<char>((char*)&obj[0], (char*)&obj[0]+obj.size()*sizeof(T));
}

template<>
vector<char> serialize(vector<char>& obj) {
    return obj;
}

template<typename T>
vector<char> serialize(list<T>& obj) {
    vector<char> v;
    for (auto& c : obj) {
        v.insert(v.end(), (char*)&c, (char*)&c+sizeof(T));
    }
    return v;
}

vector<char> serialize(string& obj) {
    return vector<char>(obj.begin(), obj.end());
}

template<typename T>
T deserialize(vector<char>& binary){
    return *(T*)binary.data();
}

template<typename T>
vector<T> deserializeVector(vector<char>& binary) {
    return vector<T>((T*)&binary[0], (T*)&binary[binary.size()]);
}

template<>
vector<char> deserializeVector(vector<char>& binary) {
    return binary;
}

string deserializeString(vector<char>& binary) {
    return binary.data();
}

template<typename T>
list<T> deserializeList(vector<char>& binary) {
    list<T> list;
    for (int i = 0; i < binary.size(); i+=sizeof(T)) {
        list.push_back(*(T*)&binary[i]);
    }
    return list;
}

string& appendVector(string& str, vector<char>& binary) {
    for (auto& b : binary) {
        str += b;
    }
}

#endif //UTILS_SERIALIZE_H
