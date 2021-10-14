//
// Created by ltc on 2021/10/14.
//

#ifndef REDIS_REDIS_H
#define REDIS_REDIS_H

#include <hiredis/hiredis.h>
#include <string>
#include <atomic>
#include "my_pthread/include/Condition.h"
#include "resource/include/ResourceSystem.h"
#include "log/include/LogSystem.h"
#include "config_system/include/ConfigSystem.h"

using std::string;

struct RedisConnection {
	redisContext* conn;
	shared_ptr<RedisConnection> next = nullptr;
};

class Redis : public Resource {
public:
	explicit Redis(const string& host, int port=6379);
	void connect();
	void close();
	shared_ptr<redisReply> executeCommand(const string& command);
	~Redis() override;
private:
	shared_ptr<RedisConnection> getConnection();
	void freeConnection(shared_ptr<RedisConnection> conn);
	void increase() override;
	void checkOut() override;
private:
	const string host;
	const int port;
	int initConnNum;
	int maxConnNum;
	shared_ptr<RedisConnection> free;
	Mutex mutex;
	std::atomic<int> connNum;
	Condition condition;
};


#endif //REDIS_REDIS_H
