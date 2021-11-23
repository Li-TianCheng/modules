//
// Created by ltc on 2021/11/21.
//

#include "raft/include/RaftServer.h"

RaftServer::RaftServer(shared_ptr<EventSystem> raft) : raft(raft), bufferChunkSize(ConfigSystem::getConfig()["system"]["net"]["read_buffer_chunk_size"].asInt()) {

}

shared_ptr<TcpSession> RaftServer::getSession() {
	return ObjPool::allocate<RaftSession>(raft.lock(), bufferChunkSize);
}
