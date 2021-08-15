//
// Created by ltc on 2021/8/4.
//

#include "config_system/include/ConfigSystem.h"

char* ConfigSystem::path = nullptr;

void ConfigSystem::init(const string &path) {
    ConfigSystem::path = realpath(path.data(), nullptr);
    std::cout << "config path: " << ConfigSystem::path << std::endl;
    Json::Reader reader;
    std::ifstream stream(path);
    if (!stream.is_open()){
        throw std::runtime_error("配置文件加载错误");
    }
    if (!reader.parse(stream, getConfig())) {
        throw std::runtime_error("配置文件解析错误");
    }
    stream.close();
    std::cout << getConfig().toStyledString() << std::endl;
}

Json::Value& ConfigSystem::getConfig() {
    static Json::Value config;
    return config;
}