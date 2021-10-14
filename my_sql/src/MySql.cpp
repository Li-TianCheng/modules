//
// Created by ltc on 2021/6/6.
//

#include "MySql.h"

MySql::MySql(const string &userName, const string &password, const string &dataBase, const string &host,
             int port) : userName(userName), password(password), initConnNum(ConfigSystem::getConfig()["system"]["my_sql"]["init_conn_num"].asInt()),
             maxConnNum(ConfigSystem::getConfig()["system"]["my_sql"]["max_conn_num"].asInt()), dataBase(dataBase), host(host), port(port), free(nullptr), connNum(0) {
    mysql_library_init;
}

void MySql::close() {
    ResourceSystem::unregisterResource(shared_from_this());
    LOG(Info, "MySql close");
}

void MySql::connect() {
    increase();
    auto time = ConfigSystem::getConfig()["system"]["my_sql"]["check_time"];
    ResourceSystem::registerResource(shared_from_this(), time[0].asInt(), time[1].asInt(), time[2].asInt(), time[3].asInt());
    LOG(Info, "MySql begin, userName:"+userName);
}

shared_ptr<Connection> MySql::getConnection() {
    mutex.lock();
    while (free == nullptr) {
        ResourceSystem::receiveEvent(EventIncrease, shared_from_this());
        condition.wait(mutex);
    }
    shared_ptr<Connection> conn = free;
    free = free->next;
    condition.notify(mutex);
	mysql_ping(&conn->conn);
    return conn;
}

void MySql::checkOut() {
    if (free == nullptr || connNum == initConnNum) {
        return;
    }
    mutex.lock();
    int target = connNum-std::max(connNum-initConnNum, initConnNum);
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
    LOG(Info, "MySql decrease, current num:"+std::to_string(connNum));
}

void MySql::increase() {
    if (free != nullptr || connNum >= maxConnNum) {
        return;
    }
    mutex.lock();
    for (int i = 0; i < initConnNum; i++) {
        auto conn = ObjPool::allocate<Connection>();
        mysql_init(&conn->conn);
		bool flag = true;
	    mysql_options(&conn->conn, MYSQL_OPT_RECONNECT, &flag);
        if (mysql_real_connect(&conn->conn, host.data(), userName.data(), password.data(), dataBase.data(), port, nullptr, CLIENT_MULTI_STATEMENTS) == nullptr) {
	        LOG(Error, "MySql create connect failed["+string(mysql_error(&conn->conn))+"]");
            mysql_close(&conn->conn);
            throw std::runtime_error("数据库连接创建失败");
        }
        conn->next = free;
        free = conn;
        connNum++;
    }
    condition.notify(mutex);
    LOG(Info, "MySql increase, current num:"+std::to_string(connNum));
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
        freeConnection(conn);
        LOG(Warn, "executeSQL failed["+sql+"] reason["+string(mysql_error(&conn->conn))+"]");
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
    LOG(Info, "executeSQL success["+sql+"]");
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
    mysql_library_end();
}

vector<vector<unordered_map<string, string>>> MySql::queryData(const string &sql) {
    shared_ptr<Connection> conn = getConnection();
    if (mysql_real_query(&conn->conn, sql.data(), sql.size()) != 0) {
        freeConnection(conn);
        LOG(Warn, "queryData failed["+sql+"] reason["+string(mysql_error(&conn->conn))+"]");
        return {};
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
    LOG(Info, "queryData success["+sql+"]");
    return result;
}
