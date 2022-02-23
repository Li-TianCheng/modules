//
// Created by ltc on 2021/11/21.
//

#ifndef RAFT_RAFTSESSION_H
#define RAFT_RAFTSESSION_H

#include "net/include/Session.h"
#include "utils/include/StringUtils.h"

struct RaftLog;

class RaftSession : public Session {
public:
	RaftSession(weak_ptr<EventSystem> raft, int bufferChunkSize);
	void handleReadDone(iter pos, size_t n) override;
	void sessionInit() override;
private:
	friend class Raft;
private:
	weak_ptr<EventSystem> raft;
	shared_ptr<string> send;
	shared_ptr<RaftLog> match;
	string ip;
	string cmd;
};


#endif //RAFT_RAFTSESSION_H
