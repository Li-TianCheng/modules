//
// Created by ltc on 2021/11/19.
//

#include "Raft.h"

Raft::Raft() : listener(ObjPool::allocate<Listener>()), state(Follower), currTerm(0), voteNum(0), voted(false),
			   heartbeatTime(ConfigSystem::getConfig()["raft"]["heartbeat_time"].asInt()),
			   timeoutMax(ConfigSystem::getConfig()["raft"]["timeout_max"].asInt()),
			   timeoutMin(ConfigSystem::getConfig()["raft"]["timeout_min"].asInt()),
			   hostIp(ConfigSystem::getConfig()["raft"]["host"].asString()),
			   snapshotPath(ConfigSystem::getConfig()["raft"]["snapshot_path"].asString()) {
	head = new RaftLog(0, 0, "init", "init");
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
	logFile.open(ConfigSystem::getConfig()["raft"]["log_path"].asString(), std::ios::out | std::ios::trunc);
}

void Raft::addServer(int port, AddressType addressType, shared_ptr<TcpServerBase> server) {
	listener->registerListener(port, addressType, server);
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
		auto* log = new RaftLog(last->idx+1, currTerm, type, cmd);
		last->next = log;
		log->prev = last;
		last = log;
		replicate();
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

void Raft::registerFuncHandler(const string &type, void (*handler)(const string&)) {
	funcHandler[type] = handler;
}

tuple<unsigned long, bool>
Raft::appendEntries(unsigned long term, const string& ip, unsigned long prevLogIdx, unsigned long prevLogTerm, vector<RaftLog*> entries,
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
	if (prevLogIdx < last->idx) {
		while (last->idx != prevLogIdx) {
			RaftLog* curr = last;
			last = last->prev;
			last->next = nullptr;
			delete curr;
		}
	}
	if (prevLogIdx == last->idx && prevLogTerm == last->term) {
		for (auto& e : entries) {
			logFile << (string)*e << std::endl;
			last->next = e;
			e->prev = last;
			last = last->next;
		}
		while (commit->idx != leaderCommit) {
			commit = commit->next;
		}
		apply();
		condition.notifyAll(mutex);
		return {currTerm, true};
	}
	if (prevLogIdx == last->idx && prevLogTerm != last->term) {
		RaftLog* curr = last;
		last = last->prev;
		last->next = nullptr;
		delete curr;
	}
	condition.notifyAll(mutex);
	return {currTerm, false};
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
	auto arg = ObjPool::allocate<addNewSessionArg>();
	arg->listener = listener;
	arg->session = session;
	arg->address = address;
	arg->addressType = IPV4;
	auto e = ObjPool::allocate<Event>(EventAddSession, arg);
	listener->receiveEvent(e);
}

void Raft::sendRequestVoteRpc(const string& address) {
	auto cmd = ObjPool::allocate<string>();
	*cmd += "$requestVote$"+to_string(currTerm)+"$"+hostIp+"$"+to_string(last->idx)+"$"+to_string(last->term)+"$$";
	auto session = raftServer->getSession();
	static_pointer_cast<RaftSession>(session)->send = cmd;
	auto arg = ObjPool::allocate<addNewSessionArg>();
	arg->listener = listener;
	arg->session = session;
	arg->address = address;
	arg->addressType = IPV4;
	auto e = ObjPool::allocate<Event>(EventAddSession, arg);
	listener->receiveEvent(e);
}

void Raft::handleTimerTimeOut(shared_ptr<void> arg) {
	auto time = static_pointer_cast<Time>(arg);
	auto raft = static_pointer_cast<Raft>(time->ePtr.lock());
	raft->mutex.lock();
	if (raft != nullptr && raft->time == time) {
		cout << raft->state << " " << raft->currTerm << endl;
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

void Raft::appendEntriesReply(unsigned long term, bool success, const string &ip, RaftLog* match) {
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
			nextIdx[ip] = nextIdx[ip]->prev;
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
	while (head != nullptr) {
		auto tmp = head;
		head = head->next;
		delete tmp;
	}
	logFile.close();
}

