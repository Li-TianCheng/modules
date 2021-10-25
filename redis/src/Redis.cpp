//
// Created by ltc on 2021/10/14.
//

#include "redis/include/Redis.h"

Redis::Redis(const string &host, int port) : host(host), port(port), free(nullptr), connNum(0),
											initConnNum(ConfigSystem::getConfig()["system"]["redis"]["init_conn_num"].asInt()),
											maxConnNum(ConfigSystem::getConfig()["system"]["redis"]["max_conn_num"].asInt()){

}

void Redis::connect() {
	increase();
	auto time = ConfigSystem::getConfig()["system"]["redis"]["check_time"];
	ResourceSystem::registerResource(shared_from_this(), time[0].asInt(), time[1].asInt(), time[2].asInt(), time[3].asInt());
	LOG(Info, "Redis begin, host:"+host+":"+std::to_string(port));
}

void Redis::close() {
	ResourceSystem::unregisterResource(shared_from_this());
	LOG(Info, "Redis close");
}

shared_ptr<RedisConnection> Redis::getConnection() {
	mutex.lock();
	while (free == nullptr) {
		ResourceSystem::receiveEvent(EventIncrease, shared_from_this());
		condition.wait(mutex);
	}
	shared_ptr<RedisConnection> conn = free;
	free = free->next;
	condition.notify(mutex);
	auto reply = (redisReply*)redisCommand(conn->conn, "PING");
	if (reply->type == REDIS_REPLY_ERROR) {
		redisReconnect(conn->conn);
	}
	freeReplyObject(reply);
	return conn;
}

void Redis::freeConnection(shared_ptr<RedisConnection> conn) {
	mutex.lock();
	conn->next = free;
	free = conn;
	condition.notify(mutex);
}

void Redis::increase() {
	if (free != nullptr || connNum >= maxConnNum) {
		return;
	}
	mutex.lock();
	for (int i = 0; i < initConnNum; i++) {
		auto conn = ObjPool::allocate<RedisConnection>();
		conn->conn = redisConnect(host.data(), port);
		if (conn->conn->err != 0) {
			LOG(Error, "Redis create connect failed["+string(conn->conn->errstr)+"]");
			break;
		} else if (conn->conn == nullptr) {
			LOG(Error, "Redis create connect failed[Can't allocate redis connect]");
			break;
		}
		conn->next = free;
		free = conn;
		connNum++;
	}
	condition.notify(mutex);
	LOG(Info, "Redis increase, current num:"+std::to_string(connNum));
}

void Redis::checkOut() {
	if (free == nullptr || connNum == initConnNum) {
		return;
	}
	mutex.lock();
	int target = connNum-std::max(connNum-initConnNum, initConnNum);
	for (int i = 0; i < target; i++) {
		if (free == nullptr) {
			break;
		}
		shared_ptr<RedisConnection> temp = free;
		free = free->next;
		redisFree(temp->conn);
		temp->next = nullptr;
		connNum--;
	}
	condition.notify(mutex);
	LOG(Info, "Redis decrease, current num:"+std::to_string(connNum));
}

Redis::~Redis() {
	while (connNum != 0) {
		mutex.lock();
		while (free != nullptr) {
			shared_ptr<RedisConnection> temp = free;
			free = free->next;
			redisFree(temp->conn);
			temp->next = nullptr;
			connNum--;
		}
		condition.notify(mutex);
	}
}

shared_ptr<redisReply> Redis::executeCommand(const string &command) {
	shared_ptr<RedisConnection> conn = getConnection();
	shared_ptr<redisReply> result((redisReply*)redisCommand(conn->conn, command.data()), freeReplyObject);
	if (result->type == REDIS_REPLY_ERROR) {
		freeConnection(conn);
		LOG(Warn, "Redis execute command failed["+command+"] reason["+result->str+"]");
		return nullptr;
	}
	freeConnection(conn);
	LOG(Info, "Redis execute command success["+command+"]");
	return result;
}
