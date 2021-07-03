//
// Created by ltc on 2021/6/7.
//

#ifndef UTILS_SERIALIZE_H
#define UTILS_SERIALIZE_H

#include <string>
#include <vector>

using namespace std;

string strToHex(const vector<unsigned char>& binary) {
    string s;
    for (auto& c : binary) {
        s += (char)((c  >> 4) + 0x30);
        s += (char)((c & 0x0f) + 0x30);
    }
    return s;
}

vector<unsigned char> hexToStr(const string& str) {
    vector<unsigned char> v;
    for (int i = 0; i < str.size(); i+=2){
        unsigned char left = (unsigned char)str[i] << 4;
        unsigned char right = ((unsigned char)str[i+1] & 0x0f);
        v.push_back(left|right);
    }
    return v;
}

template<typename T>
string serialize(T& obj) {
    return strToHex(vector<unsigned char>((unsigned char*)&obj, (unsigned char*)&obj+sizeof(T)));
}

template<typename T>
string serialize(vector<T>& obj) {
    return strToHex(vector<unsigned char>((unsigned char*)&obj[0], (unsigned char*)&obj[0]+obj.size()*sizeof(T)));
}

template<>
string serialize(vector<unsigned char>& obj) {
    return string(obj.data(), obj.data()+obj.size());
}

template<typename T>
string serialize(list<T>& obj) {
    vector<unsigned char> v;
    for (auto& c : obj) {
        v.insert(v.end(), (unsigned char*)&c, (unsigned char*)&c+sizeof(T));
    }
    return strToHex(v);
}

string serialize(string& obj) {
    return obj;
}

template<typename T>
T deserialize(const string& str){
    vector<unsigned char> binary = hexToStr(str);
    return *(T*)binary.data();
}

template<typename T>
vector<T> deserializeVector(const string& str) {
    vector<unsigned char> binary = hexToStr(str);
    return vector<T>((T*)&binary[0], (T*)&binary[binary.size()]);
}

template<>
vector<unsigned char> deserializeVector(const string& str) {
    return hexToStr(str);
}

string deserializeString(const string& str) {
    return str;
}

template<typename T>
list<T> deserializeList(const string& str) {
    list<T> list;
    vector<unsigned char> binary = hexToStr(str);
    for (int i = 0; i < binary.size(); i+=sizeof(T)) {
        list.push_back(*(T*)&binary[i]);
    }
    return list;
}

#endif //UTILS_SERIALIZE_H
