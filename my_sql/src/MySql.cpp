//
// Created by ltc on 2021/6/6.
//

#include "MySql.h"

MySql::MySql(const string &userName, const string &password, const string &dataBase, const string &host,
             int port) : userName(userName), password(password), checkTime(nullptr),
             dataBase(dataBase), host(host), port(port), free(nullptr), connNum(0) {
    init();
}

void MySql::init() {
    registerEvent(EventEndCycle, nullptr);
    registerEvent(EventTickerTimeOut, handleTimeOut);
    registerEvent(EventIncreasePool, handleIncreasePool);
}

void MySql::cycleClear() {
    checkTime->ePtr = nullptr;
    TimeSystem::deleteTicker(checkTime);
}

void MySql::handleTimeOut(shared_ptr<void> arg) {
    ((MySql*)(static_pointer_cast<Time>(arg))->ePtr)->decreasePool();
}

void MySql::close() {
    auto e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    receiveEvent(e);
}

void MySql::connect() {
    increasePool();
    auto arg = ObjPool::allocate<MySql*>(this);
    TaskSystem::addTask(cycleTask, arg);
    checkTime = ObjPool::allocate<Time>(0, 0, 1, 0, this);
    TimeSystem::receiveEvent(EventTicker, checkTime);
}

shared_ptr<Connection> MySql::getConnection() {
    mutex.lock();
    while (free == nullptr) {
        auto arg = ObjPool::allocate<MySql*>(this);
        auto e = ObjPool::allocate<Event>(EventIncreasePool, arg);
        receiveEvent(e);
        condition.wait(mutex);
    }
    shared_ptr<Connection> conn = free;
    free = free->next;
    condition.notify(mutex);
    return conn;
}

void MySql::handleIncreasePool(shared_ptr<void> arg) {
    (*static_pointer_cast<MySql*>(arg))->increasePool();
}

void MySql::decreasePool() {
    if (free == nullptr || connNum == InitConnNum) {
        return;
    }
    mutex.lock();
    int target = connNum-std::max(connNum-InitConnNum, InitConnNum);
    for (int i = 0; i < target; i++) {
        if (free == nullptr) {
            break;
        }
        shared_ptr<Connection> temp = free;
        free = free->next;
        mysql_close(&temp->conn);
        temp->next = nullptr;
        connNum--;
    }
    condition.notify(mutex);
}

void MySql::increasePool() {
    if (free != nullptr || connNum >= MaxConnNum) {
        return;
    }
    mutex.lock();
    for (int i = 0; i < InitConnNum; i++) {
        auto conn = ObjPool::allocate<Connection>();
        mysql_init(&conn->conn);
        if (mysql_real_connect(&conn->conn, host.data(), userName.data(), password.data(), dataBase.data(), port, nullptr, CLIENT_MULTI_STATEMENTS) == nullptr) {
            std::cerr << mysql_error(&conn->conn) << std::endl;
            mysql_close(&conn->conn);
            throw std::runtime_error("数据库连接创建失败");
        }
        conn->next = free;
        free = conn;
        connNum++;
    }
    condition.notify(mutex);
}

void MySql::freeConnection(shared_ptr<Connection> conn) {
    mutex.lock();
    conn->next = free;
    free = conn;
    condition.notify(mutex);
}

bool MySql::executeSQL(const string &sql) {
    shared_ptr<Connection> conn = getConnection();
    if (mysql_real_query(&conn->conn, sql.data(), sql.size()) != 0) {
        std::cerr << mysql_error(&conn->conn) << std::endl;
        freeConnection(conn);
        return false;
    }
    while (true) {
        MYSQL_RES* re = mysql_store_result(&conn->conn);
        mysql_free_result(re);
        if (mysql_next_result(&conn->conn) != 0) {
            break;
        }
    }
    freeConnection(conn);
    return true;
}

MySql::~MySql() {
    while (connNum != 0) {
        mutex.lock();
        while (free != nullptr) {
            shared_ptr<Connection> temp = free;
            free = free->next;
            mysql_close(&temp->conn);
            temp->next = nullptr;
            connNum--;
        }
        condition.notify(mutex);
    }
}

vector<vector<unordered_map<string, string>>> MySql::queryData(const string &sql) {
    shared_ptr<Connection> conn = getConnection();
    if (mysql_real_query(&conn->conn, sql.data(), sql.size()) != 0) {
        std::cerr << mysql_error(&conn->conn) << std::endl;
        freeConnection(conn);
        return vector<vector<unordered_map<string, string>>>();
    }
    vector<vector<unordered_map<string, string>>> result;
    while (true) {
        MYSQL_RES* re = mysql_store_result(&conn->conn);
        MYSQL_FIELD* field = mysql_fetch_field(re);
        vector<unordered_map<string, string>> temp;
        while (true) {
            MYSQL_ROW row = mysql_fetch_row(re);
            if (!row) {
                break;
            }
            unordered_map<string, string> map;
            for (int i = 0; i < mysql_num_fields(re); i++) {
                if (row[i] == nullptr) {
                    map[field[i].name] = "NULL";
                } else {
                    map[field[i].name] = row[i];
                }
            }
            temp.push_back(map);
        }
        mysql_free_result(re);
        result.push_back(temp);
        if (mysql_next_result(&conn->conn) != 0) {
            break;
        }
    }
    freeConnection(conn);
    return result;
}
