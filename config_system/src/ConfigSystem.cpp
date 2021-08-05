//
// Created by ltc on 2021/8/4.
//

#include "config_system/include/ConfigSystem.h"

void ConfigSystem::init(const string &path) {
    Json::Reader reader;
    std::ifstream stream(path);
    if (!stream.is_open()){
        throw std::runtime_error("配置文件加载错误");
    }
    if (!reader.parse(stream, getConfig())) {
        throw std::runtime_error("配置文件解析错误");
    }
    stream.close();
    std::cout << "config path: " << path << std::endl;
    std::cout << getConfig().toStyledString() << std::endl;
}

Json::Value& ConfigSystem::getConfig() {
    static Json::Value config;
    return config;
}