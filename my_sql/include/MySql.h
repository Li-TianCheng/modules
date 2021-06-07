//
// Created by ltc on 2021/6/6.
//

#ifndef MYSQL_MYSQL_H
#define MYSQL_MYSQL_H

#include <string>
#include <list>
#include <queue>
#include <mysql/mysql.h>
#include "event_system/include/EventSystem.h"
#include "time_system/include/TimeSystem.h"
#include "task_system/include/TaskSystem.h"

using std::string;
using std::list;
using std::queue;

static const int InitConnNum = 10;
static const int MaxConnNum  = 100;

struct Connection {
    MYSQL conn;
    Connection* next = nullptr;
};

struct QueryData {
    MYSQL_RES* result;
    Connection* conn;
    QueryData(MYSQL_RES* result, Connection* conn) : result(result), conn(conn) {};
};

class MySql : public EventSystem{
public:
    MySql(const string &userName, const string &password, const string &dataBase, const string& host="localhost", int port=3306);
    void connect();
    void close();
    void executeSQL(const string& sql);
    QueryData* queryData(const string& sql);
    void freeQueryData(QueryData* result);
    ~MySql() override;
private:
    Connection* getConnection();
    void freeConnection(Connection* conn);
    void increasePool();
    void decreasePool();
    void init();
    void cycleClear() override;
    static void handleTimeOut(void* arg);
    static void handleIncreasePool(void* arg);
private:
    const string userName;
    const string password;
    const string dataBase;
    const string host;
    const int port;
    Connection* free;
    Time* checkTime;
    string uuid;
    Mutex mutex;
    volatile int connNum;
    Condition condition;
};

#endif //MYSQL_MYSQL_H
