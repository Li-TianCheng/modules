//
// Created by ltc on 2021/11/19.
//

#ifndef RAFT_RAFT_H
#define RAFT_RAFT_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <ctime>
#include <fstream>
#include "time_system/include/Time.h"
#include "event_system/include/EventSystem.h"
#include "net/include/Listener.h"
#include "RaftSession.h"
#include "RaftServer.h"

using std::vector;
using std::queue;
using std::unordered_map;
using std::tuple;

enum RaftState {
	Follower,
	Candidate,
	Leader
};

struct RaftLog {
	unsigned long idx;
	unsigned long term;
	string type;
	string cmd;
	RaftLog* next = nullptr;
	RaftLog* prev = nullptr;
	RaftLog(unsigned long idx, unsigned long term, const string& type, const string& cmd) : idx(idx), term(term), type(type), cmd(cmd) {}
	explicit RaftLog(const string& s) {
		vector<string> split = utils::split(s, '#');
		idx = stoul(split[0]);
		term = stoul(split[1]);
		type = split[2];
		cmd = split[3];
	}
	explicit operator string() const {
		return "#" + to_string(idx) + "#" + to_string(term) + "#" + type + "#" + cmd + "##";
	}
};

class Raft : public EventSystem {
public:
	Raft();
	void addServer(int port, AddressType addressType, shared_ptr<TcpServerBase> server);
	void serve();
	string startCmd(const string& type, const string& cmd);
	void registerFuncHandler(const string& type, void(*handler)(const string&));
	~Raft() override;
private:
	static void handleTimerTimeOut(shared_ptr<void> arg);
	void replicate();
	void election();
	void apply();
	void sendAppendEntriesRpc(const string& address);
	void sendRequestVoteRpc(const string& address);
	tuple<unsigned long, bool> appendEntries(unsigned long term, const string& ip, unsigned long prevLogIdx, unsigned long prevLogTerm, vector<RaftLog*> entries, unsigned long leaderCommit);
	tuple<int, bool> requestVote(unsigned long term, unsigned long lastLogIdx, unsigned long lastLogTerm);
	void appendEntriesReply(unsigned long term, bool success, const string& ip, RaftLog* match);
	void requestVoteReply(unsigned long term, bool success);
private:
	friend class RaftSession;
private:
	RaftState state;
	int timeoutMax;
	int timeoutMin;
	int heartbeatTime;
	int voteNum;
	unsigned long currTerm;
	bool voted;
	shared_ptr<Time> time;
	RaftLog* last;
	RaftLog* head;
	RaftLog* commit;
	RaftLog* lastApplied;
	unordered_map<string, RaftLog*> nextIdx;
	unordered_map<string, RaftLog*> matchIdx;
	unordered_map<string, void(*)(const string&)> funcHandler;
	Mutex mutex;
	Condition condition;
	shared_ptr<Listener> listener;
	shared_ptr<RaftServer> raftServer;
	vector<string> clusterAddress;
	string hostIp;
	string leaderIp;
	std::ofstream logFile;
	string snapshotPath;
};


#endif //RAFT_RAFT_H
