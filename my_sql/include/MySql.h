//
// Created by ltc on 2021/6/6.
//

#ifndef MYSQL_MYSQL_H
#define MYSQL_MYSQL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mysql/mysql.h>
#include "resource/include/ResourceSystem.h"
#include "time_system/include/TimeSystem.h"
#include "task_system/include/TaskSystem.h"

using std::string;
using std::queue;
using std::vector;
using std::unordered_map;

static const int InitConnNum = 10;
static const int MaxConnNum  = 100;

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
    ~MySql();
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
    shared_ptr<Connection> free;
    shared_ptr<Time> checkTime;
    Mutex mutex;
    std::atomic<int> connNum;
    Condition condition;
};

#endif //MYSQL_MYSQL_H
