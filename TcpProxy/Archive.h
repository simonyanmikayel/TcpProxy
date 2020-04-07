#pragma once

#include "LogData.h"

const size_t MAX_BUF_SIZE = 1024 * 1024 * 1024;

class Archive
{
public:
	Archive();
	~Archive();
	void lock();
	void unlock();
	void clearArchive();
	DWORD getNodeCount() { return m_pNodes ? m_pNodes->Count() : 0; }
	LOG_NODE* getNode(DWORD i) { return (m_pNodes && i < m_pNodes->Count()) ? (LOG_NODE*)m_pNodes->Get(i) : 0; }
	ROOT_NODE* getRootNode() { return m_rootNode; }
	ROUTER_NODE* addRouter(const Router* pRouter);
	ROUTER_NODE* getRouter(const Router* pRouter);
	CONN_NODE* addConnection(const Connection* pConnection);
	CONN_NODE* getConnection(const Connection* pConnection);
	RECV_NODE* addRecv(const Socket* pSocket);
	char* Alloc(DWORD cb, bool zero = false) { return (char*)m_pTraceBuf->Alloc(cb, zero); }
private:
	PtrArray<LOG_NODE>* m_pNodes;
	ROOT_NODE* m_rootNode;
	MemBuf* m_pTraceBuf;
	CRITICAL_SECTION m_cs;
	static DWORD archiveNumber;
};

extern Archive gArchive;

struct SYNC
{
	SYNC() { gArchive.lock(); }
	~SYNC() { gArchive.unlock(); }
};

