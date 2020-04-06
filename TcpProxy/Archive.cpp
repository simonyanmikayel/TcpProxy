#include "stdafx.h"
#include "Archive.h"
#include "Helpers.h"

DWORD Archive::archiveNumber = 0;
Archive    gArchive;

Archive::Archive()
{
    ZeroMemory(this, sizeof(*this));
    m_pTraceBuf = new MemBuf(MAX_BUF_SIZE, 256 * 2 * 1024 * 1024);
    InitializeCriticalSectionAndSpinCount(&m_cs, 0x00000400);
    clearArchive();
}

Archive::~Archive()
{
    //we intentionaly do not free memory for this single instance to allow application quick exit
    //delete m_pMemBuf;
    DeleteCriticalSection(&m_cs);
}

void Archive::lock()
{
    EnterCriticalSection(&m_cs);
}

void Archive::unlock()
{
    LeaveCriticalSection(&m_cs);
}

void Archive::clearArchive()
{
    SYNC sync;
    archiveNumber++;

    delete m_pNodes;
    m_pNodes = nullptr;
    m_rootNode = nullptr;
    m_pTraceBuf->Free();

    m_pNodes = new PtrArray<LOG_NODE>(m_pTraceBuf);
    m_rootNode = (ROOT_NODE*)m_pNodes->Add(sizeof(ROOT_NODE), true);
    m_rootNode->data_type = LOG_TYPE::ROOT;
    ATLASSERT(m_pNodes && m_rootNode);
}

ROUTER_NODE* Archive::getRouter(const Router* pRouter)
{
    SYNC sync;
    ROUTER_NODE* pNode = (ROUTER_NODE*)gArchive.getRootNode()->lastChild;
    while (pNode)
    {
        if (pNode->id == pRouter->ID())
            return pNode;
        pNode = (ROUTER_NODE*)pNode->prevSibling;
    }
    return nullptr;
}

ROUTER_NODE* Archive::addRouter(const Router* pRouter)
{
    SYNC sync;
    ROUTER_NODE* pNode = getRouter(pRouter);
    if (pNode == nullptr)
    {
        const char* name = pRouter->GetRote()->name.c_str();
        WORD cb_name = (WORD)strlen(name);
        pNode = (ROUTER_NODE*)m_pNodes->Add(sizeof(ROUTER_NODE) + cb_name, true);
        if (!pNode)
            return nullptr;

        pNode->data_type = LOG_TYPE::ROUTER;
        pNode->id = pRouter->ID();
        pNode->cb_name = cb_name;
        memcpy(pNode->name(), name, cb_name);
        getRootNode()->add_child(pNode);
    }
    return pNode;
}

CONN_NODE* Archive::getConnection(const ROUTER_NODE* pRouterNode, const Connection* pConnection)
{
    SYNC sync;
    CONN_NODE* pNode = (CONN_NODE*)pRouterNode->lastChild;
    while (pNode)
    {
        if (pNode->id == pConnection->ID())
            return pNode;
        pNode = (CONN_NODE*)pNode->prevSibling;
    }
    return nullptr;
}

CONN_NODE* Archive::addConnection(const Connection* pConnection)
{
    SYNC sync;
    Router* pRouter = pConnection->m_pRouter;
    ROUTER_NODE* pRouterNode = addRouter(pRouter);
    if (!pRouterNode)
        return nullptr;
    CONN_NODE* pNode = getConnection(pRouterNode, pConnection);

    if (pNode == nullptr)
    {
        pNode = (CONN_NODE*)m_pNodes->Add(sizeof(CONN_NODE), true);
        if (!pNode)
            return nullptr;

        pNode->data_type = LOG_TYPE::CONN;
        pNode->id = pConnection->ID();
        pRouterNode->add_child(pNode);
    }
    return pNode;
}

RECV_NODE* Archive::addRecv(const Socket* pSocket)
{
    SYNC sync;
    Connection* pConnection = pSocket->m_pConnection;

    CONN_NODE* pConnNode = addConnection(pConnection);
    if (!pConnNode)
        return nullptr;

    RECV_NODE* pNode = (RECV_NODE*)m_pNodes->Add(sizeof(RECV_NODE), true);
    if (!pNode)
        return nullptr;

    pNode->data_type = LOG_TYPE::RECV;
    pNode->isLocal = (pConnection->SocketType(pSocket) == SOCKET_TYPE::ACCEPT);
    pConnNode->add_child(pNode);
    return pNode;
}
