//
// Created by ltc on 2021/11/19.
//

#include "Raft.h"

Raft::Raft() : listener(ObjPool::allocate<Listener>()), state(Follower), currTerm(0), voteNum(0), voted(false), logFileLen(0),
               mergeNum(ConfigSystem::getConfig()["raft"]["merge_replication_num"].asInt()),
			   heartbeatTime(ConfigSystem::getConfig()["raft"]["heartbeat_time"].asInt()),
			   timeoutMax(ConfigSystem::getConfig()["raft"]["timeout_max"].asInt()),
			   timeoutMin(ConfigSystem::getConfig()["raft"]["timeout_min"].asInt()),
			   hostIp(ConfigSystem::getConfig()["raft"]["host"].asString()),
			   logPath(ConfigSystem::getConfig()["raft"]["log_path"].asString()),
			   snapshotPath(ConfigSystem::getConfig()["raft"]["snapshot_path"].asString()) {
	head = ObjPool::allocate<RaftLog>(0, 0, "init", "init");
	last = head;
	commit = head;
	lastApplied = head;
	auto cluster = ConfigSystem::getConfig()["raft"]["cluster"];
	for (auto& c : cluster) {
		clusterAddress.push_back(c.asString());
		matchIdx[c.asString()] = head;
		nextIdx[c.asString()] = head;
	}
	srand(::time(nullptr));
	logFile.open(logPath, std::ios::in|std::ios::out|std::ios::app);
	string tmp;
	while (getline(logFile, tmp)) {
		if (tmp.size() > 2 && tmp[tmp.size()-2] == '#' && tmp[tmp.size()-1] == '#') {
			logFileLen += tmp.size()+1;
			auto log = ObjPool::allocate<RaftLog>(tmp);
			last->next = log;
			log->prev = last;
			last = last->next;
		} else {
			truncate(logPath.data(), logFileLen);
			break;
		}
	}
	currTerm = last->term;
	logFile.clear();
}

shared_ptr<Listener> Raft::getListener() {
	return listener;
}

void Raft::serve() {
	raftServer = ObjPool::allocate<RaftServer>(shared_from_this());
	listener->registerListener(ConfigSystem::getConfig()["raft"]["port"].asInt(), IPV4, raftServer);
	registerEvent(EventTimerTimeOut, handleTimerTimeOut);
	registerEvent(EventEndCycle, nullptr);
	TaskSystem::addTask(cycleTask, shared_from_this());
	int ms = rand() % (timeoutMax-timeoutMin) + timeoutMin;
	time = ObjPool::allocate<Time>(0, 0, 0, ms, shared_from_this());
	TimeSystem::receiveEvent(EventTimer, time);
	listener->listen();
}

string Raft::startCmd(const string& type, const string &cmd) {
	if (state == Leader) {
		mutex.lock();
		auto log = ObjPool::allocate<RaftLog>(last->idx+1, currTerm, type, cmd);
		last->next = log;
		log->prev = last;
		last = log;
		logFileLen += ((string)*log).size()+1;
		logFile.clear();
		logFile << (string)*log << endl;
		if (last->idx-commit->idx >= mergeNum) {
			replicate();
		}
		while (state != Leader || commit->idx < log->idx) {
			condition.wait(mutex);
		}
		condition.notifyAll(mutex);
		if (state == Leader && commit->idx >= log->idx) {
			return "success";
		}
		return "failed";
	}
	return leaderIp;
}

string Raft::isLeader() {
	if (state == Leader) {
		return "";
	}
	return leaderIp;
}

void Raft::registerFuncHandler(const string &type, void (*handler)(const string&)) {
	funcHandler[type] = handler;
}

tuple<unsigned long, bool>
Raft::appendEntries(unsigned long term, const string& ip, unsigned long prevLogIdx, unsigned long prevLogTerm, vector<shared_ptr<RaftLog>> entries,
                    unsigned long leaderCommit) {
	if (term < currTerm) {
		return {currTerm, false};
	}
	mutex.lock();
	currTerm = term;
	leaderIp = ip;
	state = Follower;
	int ms = rand() % (timeoutMax-timeoutMin) + timeoutMin;
	time = ObjPool::allocate<Time>(0, 0, 0, ms, shared_from_this());
	TimeSystem::receiveEvent(EventTimer, time);
	if (prevLogIdx > last->idx) {
		condition.notifyAll(mutex);
		return {currTerm, false};
	}
	auto curr = last;
	while (curr->idx != prevLogIdx) {
		curr = curr->prev.lock();
	}
	if (curr->term == prevLogTerm) {
		for (auto& e : entries) {
			if (curr->next != nullptr) {
				if (curr->next->term != e->term) {
					auto tmp = curr->next;
					while (tmp != nullptr) {
						auto t = tmp->next;
						logFileLen -= ((string)*tmp).size()+1;
						tmp = t;
					}
					truncate(logPath.data(), logFileLen);
					curr->next = e;
					e->prev = curr;
					logFileLen += ((string)*e).size()+1;
					logFile.clear();
					logFile << (string)*e << endl;
				}
			} else {
				curr->next = e;
				e->prev = curr;
				logFileLen += ((string)*e).size()+1;
				logFile.clear();
				logFile << (string)*e << endl;
			}
			curr = curr->next;
		}
		last = curr;
		logFile.flush();
		while (commit->idx < leaderCommit) {
			commit = commit->next;
		}
		apply();
		condition.notifyAll(mutex);
		return {currTerm, true};
	} else {
		while (last != curr->prev.lock()) {
			auto tmp = last->prev.lock();
			logFileLen -= ((string)*tmp).size()+1;
			last = tmp;
		}
		truncate(logPath.data(), logFileLen);
		last->next = nullptr;
		condition.notifyAll(mutex);
		return {currTerm, false};
	}
}

tuple<int, bool> Raft::requestVote(unsigned long term, unsigned long lastLogIdx, unsigned long lastLogTerm) {
	if (term < currTerm) {
		return {currTerm, false};
	}
	mutex.lock();
	if (term > currTerm) {
		state = Follower;
		currTerm = term;
		voted = false;
		int ms = rand() % (timeoutMax-timeoutMin) + timeoutMin;
		time = ObjPool::allocate<Time>(0, 0, 0, ms, shared_from_this());
		TimeSystem::receiveEvent(EventTimer, time);
	}
	if (!voted && lastLogTerm >= last->term && lastLogIdx >= last->idx) {
		voted = true;
		voteNum = 0;
		state = Candidate;
		int ms = rand() % (timeoutMax-timeoutMin) + timeoutMin;
		time = ObjPool::allocate<Time>(0, 0, 0, ms, shared_from_this());
		TimeSystem::receiveEvent(EventTimer, time);
		for (auto& address : clusterAddress) {
			sendRequestVoteRpc(address);
		}
		condition.notifyAll(mutex);
		return {currTerm, true};
	}
	condition.notifyAll(mutex);
	return {currTerm, false};
}

void Raft::replicate() {
	logFile.flush();
	for (auto& address : clusterAddress) {
		sendAppendEntriesRpc(address);
	}
	time = ObjPool::allocate<Time>(0, 0, 0, heartbeatTime, shared_from_this());
	TimeSystem::receiveEvent(EventTimer, time);
}

void Raft::election() {
	++currTerm;
	voteNum = 1;
	int ms = rand() % (timeoutMax-timeoutMin) + timeoutMin;
	time = ObjPool::allocate<Time>(0, 0, 0, ms, shared_from_this());
	TimeSystem::receiveEvent(EventTimer, time);
	for (auto& address : clusterAddress) {
		sendRequestVoteRpc(address);
	}
}

void Raft::apply() {
	while (lastApplied != commit) {
		lastApplied = lastApplied->next;
		funcHandler[lastApplied->type](lastApplied->cmd);
	}
}

void Raft::sendAppendEntriesRpc(const string& address) {
	auto curr = nextIdx[address];
	auto cmd = ObjPool::allocate<string>();
	*cmd = "$appendEntries$"+to_string(currTerm)+"$"+hostIp+"$"+to_string(curr->idx)+"$"+to_string(curr->term)+"$";
	auto log = ObjPool::allocate<string>();
	*log = "[";
	while (curr != last) {
		curr = curr->next;
		*log += (string)(*curr) + ",";
	}
	*log += "]";
	*cmd += *log+"$"+to_string(commit->idx)+"$$";
	auto session = raftServer->getSession();
	static_pointer_cast<RaftSession>(session)->match = last;
	static_pointer_cast<RaftSession>(session)->ip = address;
	static_pointer_cast<RaftSession>(session)->send = cmd;
	listener->addNewSession(session, address, IPV4);
}

void Raft::sendRequestVoteRpc(const string& address) {
	auto cmd = ObjPool::allocate<string>();
	*cmd += "$requestVote$"+to_string(currTerm)+"$"+to_string(last->idx)+"$"+to_string(last->term)+"$$";
	auto session = raftServer->getSession();
	static_pointer_cast<RaftSession>(session)->send = cmd;
	auto arg = ObjPool::allocate<addNewSessionArg>();
	listener->addNewSession(session, address, IPV4);
}

void Raft::handleTimerTimeOut(shared_ptr<void> arg) {
	auto time = static_pointer_cast<Time>(arg);
	auto raft = static_pointer_cast<Raft>(time->ePtr.lock());
	raft->mutex.lock();
	if (raft != nullptr && raft->time == time) {
		if (raft->state == Follower) {
			raft->state = Candidate;
			raft->election();
		} else if (raft->state == Candidate) {
			raft->election();
		} else {
			raft->replicate();
		}
	}
	raft->condition.notifyAll(raft->mutex);
}

void Raft::appendEntriesReply(unsigned long term, bool success, const string &ip, shared_ptr<RaftLog> match) {
	mutex.lock();
	if (term > currTerm) {
		currTerm = term;
		state = Follower;
	} else if (state == Leader && term == currTerm){
		if (success) {
			if (match->idx > matchIdx[ip]->idx) {
				matchIdx[ip] = match;
				int count = 1;
				for (auto& m : matchIdx) {
					if (m.second->idx >= match->idx) {
						++count;
					}
					if (count > (matchIdx.size()+1)/2) {
						commit = match;
						apply();
						break;
					}
				}
			}
		} else {
			if (nextIdx[ip] != head) {
				nextIdx[ip] = nextIdx[ip]->prev.lock();
			}
			sendAppendEntriesRpc(ip);
		}
	}
	condition.notifyAll(mutex);
}

void Raft::requestVoteReply(unsigned long term, bool success) {
	mutex.lock();
	if (term > currTerm) {
		currTerm = term;
		state = Follower;
	} else if (state == Candidate && term == currTerm) {
		if (success) {
			++voteNum;
			if (voteNum >= (clusterAddress.size()+1)/2+1) {
				state = Leader;
				time = ObjPool::allocate<Time>(0, 0, 0, heartbeatTime, shared_from_this());
				TimeSystem::receiveEvent(EventTimer, time);
				for (auto& address : clusterAddress) {
					nextIdx[address] = last;
					matchIdx[address] = head;
					sendAppendEntriesRpc(address);
				}
			}
		}
	}
	condition.notifyAll(mutex);
}

Raft::~Raft() {
	logFile.close();
}

