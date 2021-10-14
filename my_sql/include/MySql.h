//
// Created by ltc on 2021/6/6.
//

#ifndef MYSQL_MYSQL_H
#define MYSQL_MYSQL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>
#include "resource/include/ResourceSystem.h"
#include "time_system/include/TimeSystem.h"
#include "log/include/LogSystem.h"
#include "config_system/include/ConfigSystem.h"

using std::string;
using std::vector;
using std::unordered_map;

struct Connection {
    MYSQL conn;
    shared_ptr<Connection> next = nullptr;
};

class MySql : public Resource{
public:
    MySql(const string &userName, const string &password, const string &dataBase, const string& host="localhost", int port=3306);
    void connect();
    void close();
    bool executeSQL(const string& sql);
    vector<vector<unordered_map<string, string>>> queryData(const string& sql);
    ~MySql() override;
private:
    shared_ptr<Connection> getConnection();
    void freeConnection(shared_ptr<Connection> conn);
    void increase() override;
    void checkOut() override;
private:
    const string userName;
    const string password;
    const string dataBase;
    const string host;
    const int port;
    int initConnNum;
    int maxConnNum;
    shared_ptr<Connection> free;
    Mutex mutex;
    std::atomic<int> connNum;
    Condition condition;
};

#endif //MYSQL_MYSQL_H
