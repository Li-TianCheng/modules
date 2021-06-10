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
    TimeSystem::deleteTicker(uuid);
}

void MySql::handleTimeOut(void *arg) {
    ((MySql*)((Time*)arg)->ePtr)->decreasePool();
}

void MySql::close() {
    Event* e = ObjPool::allocate<Event>(EventEndCycle, nullptr);
    receiveEvent(e);
}

void MySql::connect() {
    increasePool();
    TaskSystem::addTask(cycleTask, this);
    checkTime = ObjPool::allocate<Time>(0, 0, 1, 0, this);
    uuid = TimeSystem::receiveEvent(EventTicker, checkTime);
}

Connection* MySql::getConnection() {
    mutex.lock();
    while (free == nullptr) {
        Event* e = ObjPool::allocate<Event>(EventIncreasePool, this);
        receiveEvent(e);
        condition.wait(mutex);
    }
    Connection* conn = free;
    free = free->next;
    condition.notifyAll(mutex);
    return conn;
}

void MySql::handleIncreasePool(void * arg) {
    ((MySql*)arg)->increasePool();
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
        Connection* temp = free;
        free = free->next;
        mysql_close(&temp->conn);
        temp->next = nullptr;
        ObjPool::deallocate(temp);
        connNum--;
    }
    condition.notifyAll(mutex);
}

void MySql::increasePool() {
    if (free != nullptr || connNum >= MaxConnNum) {
        return;
    }
    mutex.lock();
    for (int i = 0; i < InitConnNum; i++) {
        Connection* conn = ObjPool::allocate<Connection>();
        mysql_init(&conn->conn);
        if (mysql_real_connect(&conn->conn, host.data(), userName.data(), password.data(), dataBase.data(), port, nullptr, CLIENT_MULTI_STATEMENTS) == nullptr) {
            std::cerr << mysql_error(&conn->conn) << std::endl;
            mysql_close(&conn->conn);
            ObjPool::deallocate(conn);
            throw std::runtime_error("数据库连接创建失败");
        }
        conn->next = free;
        free = conn;
        connNum++;
    }
    condition.notifyAll(mutex);
}

void MySql::freeConnection(Connection *conn) {
    mutex.lock();
    for (auto& re : conn->result) {
        mysql_free_result(re);
    }
    conn->result.clear();
    conn->next = free;
    free = conn;
    condition.notifyAll(mutex);
}

void MySql::executeSQL(const string &sql) {
    Connection* conn = queryData(sql);
    freeConnection(conn);
}

MySql::~MySql() {
    while (connNum != 0) {
        mutex.lock();
        while (free != nullptr) {
            Connection* temp = free;
            free = free->next;
            mysql_close(&temp->conn);
            temp->next = nullptr;
            ObjPool::deallocate(temp);
            connNum--;
        }
        condition.notifyAll(mutex);
    }
    TimeSystem::deleteTicker(uuid);
}

Connection* MySql::queryData(const string &sql) {
    Connection* conn = getConnection();
    if (mysql_real_query(&conn->conn, sql.data(), sql.size()) != 0) {
        std::cerr << mysql_error(&conn->conn) << std::endl;
        freeConnection(conn);
        throw std::runtime_error("执行SQL失败");
    }
    MYSQL_RES* re = mysql_store_result(&conn->conn);
    conn->result.push_back(re);
    while (!mysql_next_result(&conn->conn) ) {
        MYSQL_RES* re = mysql_store_result(&conn->conn);
        conn->result.push_back(re);
    }
    return conn;
}

void MySql::freeQueryData(Connection* conn) {
    freeConnection(conn);
}
