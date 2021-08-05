//
// Created by ltc on 2021/8/4.
//

#ifndef CONFIGSYSTEM_CONFIGSYSTEM_H
#define CONFIGSYSTEM_CONFIGSYSTEM_H

#include <iostream>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>

using std::string;

class ConfigSystem {
public:
    static void init(const string& path);
    static Json::Value& getConfig();
private:
    ConfigSystem() = default;
};


#endif //CONFIGSYSTEM_CONFIGSYSTEM_H
