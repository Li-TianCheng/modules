//
// Created by ltc on 2021/11/21.
//

#include "RaftSession.h"
#include "Raft.h"

RaftSession::RaftSession(weak_ptr<EventSystem> raft, int bufferChunkSize) : raft(raft), TcpSession(bufferChunkSize) {

}

void RaftSession::handleReadDone(iter pos, size_t n) {
	for (size_t i = 0; i < n; i++) {
		cmd += *pos++;
	}
	if (cmd.size() > 2 && cmd[cmd.size()-1] == '$' && cmd[cmd.size()-2] == '$') {
		vector<string> split = utils::split(cmd, '$');
		auto r = static_pointer_cast<Raft>(raft.lock());
		if (r != nullptr) {
			if (split[0] == "appendEntries") {
				unsigned long term = stoul(split[1]);
				string leaderIp = split[2];
				unsigned long prevLogIdx = stoul(split[3]);
				unsigned long prevLogTerm = stoul(split[4]);
				vector<shared_ptr<RaftLog>> entries;
				vector<string> tmp = utils::split(split[5].substr(1, split[5].size()-2), ',');
				entries.reserve(tmp.size());
				for (auto& log : tmp) {
					entries.push_back(ObjPool::allocate<RaftLog>(log));
				}
				unsigned long leaderCommit = stoul(split[6]);
				auto res = r->appendEntries(term, leaderIp, prevLogIdx, prevLogTerm, entries, leaderCommit);
				auto reply = ObjPool::allocate<string>();
				*reply = "$appendEntriesReply$"+ to_string(get<0>(res))+"$"+to_string(get<1>(res))+"$$";
				write(reply);
			} else if (split[0] == "requestVote") {
				unsigned long term = stoul(split[1]);
				unsigned long lastLogIdx = stoul(split[2]);
				unsigned long lastLogTerm = stoul(split[3]);
				auto res = r->requestVote(term, lastLogIdx, lastLogTerm);
				auto reply = ObjPool::allocate<string>();
				*reply = "$requestVoteReply$"+ to_string(get<0>(res))+"$"+to_string(get<1>(res))+"$$";
				write(reply);
			} else if (split[0] == "appendEntriesReply") {
				unsigned long term = stoul(split[1]);
				bool success = stoi(split[2]);
				r->appendEntriesReply(term, success, ip, match);
				closeConnection();
			} else if (split[0] == "requestVoteReply") {
				unsigned long term = stoul(split[1]);
				bool success = stoi(split[2]);
				r->requestVoteReply(term, success);
				closeConnection();
			}
		}
		cmd = "";
	}
	readDone(n);
}

void RaftSession::sessionInit() {
	if (send != nullptr) {
		write(send);
	}
}

