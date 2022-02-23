//
// Created by ltc on 2021/11/21.
//

#ifndef RAFT_RAFTSERVER_H
#define RAFT_RAFTSERVER_H

#include "net/include/ServerBase.h"
#include "RaftSession.h"

class Raft;

class RaftServer : public ServerBase {
public:
	explicit RaftServer(shared_ptr<EventSystem> raft);
	shared_ptr<Session> getSession() override;
private:
	weak_ptr<EventSystem> raft;
	int bufferChunkSize;
};


#endif //RAFT_RAFTSERVER_H
