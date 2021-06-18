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
#include "event_system/include/EventSystem.h"
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

class MySql : public EventSystem{
public:
    MySql(const string &userName, const string &password, const string &dataBase, const string& host="localhost", int port=3306);
    void connect();
    void close();
    void executeSQL(const string& sql);
    vector<vector<unordered_map<string, string>>> queryData(const string& sql);
    ~MySql() override;
private:
    shared_ptr<Connection> getConnection();
    void freeConnection(const shared_ptr<Connection>& conn);
    void increasePool();
    void decreasePool();
    void init();
    void cycleClear() override;
    static void handleTimeOut(const shared_ptr<void>& arg);
    static void handleIncreasePool(const shared_ptr<void>& arg);
private:
    const string userName;
    const string password;
    const string dataBase;
    const string host;
    const int port;
    shared_ptr<Connection> free;
    shared_ptr<Time> checkTime;
    string uuid;
    Mutex mutex;
    volatile int connNum;
    Condition condition;
};

#endif //MYSQL_MYSQL_H
