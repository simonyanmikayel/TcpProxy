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

size_t Archive::UsedMemory()
{
    return m_pTraceBuf->UsedMemory();
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
    m_rootNode->expanded = 1;
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
        const char* remote_addr = pRouter->GetRote()->remote_addr.c_str();
        WORD cb_name = (WORD)strlen(name);
        WORD cb_remote_addr = (WORD)strlen(remote_addr);
        pNode = (ROUTER_NODE*)m_pNodes->Add(sizeof(ROUTER_NODE) + cb_name + 1 + cb_remote_addr + 1, true);
        if (!pNode)
            return nullptr;

        pNode->data_type = LOG_TYPE::ROUTER;
        pNode->id = pRouter->ID();
        pNode->local_port = pRouter->GetRote()->local_port;
        pNode->remote_port = pRouter->GetRote()->remote_port;
        pNode->cb_name = cb_name;
        pNode->cb_remote_addr = cb_remote_addr;
        memcpy(pNode->name(), name, cb_name);
        memcpy(pNode->remote_addr(), remote_addr, cb_remote_addr);
        getRootNode()->add_child(pNode);
    }
    return pNode;
}

CONN_NODE* Archive::getConnection(const Connection* pConnection)
{
    SYNC sync;
    Router* pRouter = pConnection->m_pRouter;
    ROUTER_NODE* pRouterNode = getRouter(pRouter);
    if (!pRouterNode)
        return nullptr;
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
    CONN_NODE* pNode = getConnection(pConnection);

    if (pNode == nullptr)
    {
        Router* pRouter = pConnection->m_pRouter;
        ROUTER_NODE* pRouterNode = addRouter(pRouter);
        if (!pRouterNode)
            return nullptr;
        pNode = (CONN_NODE*)m_pNodes->Add(sizeof(CONN_NODE), true);
        if (!pNode)
            return nullptr;

        pNode->data_type = LOG_TYPE::CONN;
        pNode->initTime = pConnection->initTime;
        pNode->id = pConnection->ID();
        pRouterNode->add_child(pNode);
    }
    return pNode;
}

EXCHANGE_NODE* Archive::addExchange(const Socket* pSocket, char* pData, DWORD cData)
{
    SYNC sync;
    Connection* pConnection = pSocket->m_pConnection;

    CONN_NODE* pConnNode = addConnection(pConnection);
    if (!pConnNode)
        return nullptr;

    EXCHANGE_NODE* pNode = (EXCHANGE_NODE*)m_pNodes->Add(sizeof(EXCHANGE_NODE) + cData, true);
    if (!pNode)
        return nullptr;

    GetLocalTime(&pNode->time);
    pNode->cData = cData;
    memcpy(pNode->data(), pData, cData);
    pNode->data_type = LOG_TYPE::EXCHANGE;
    pNode->isLocal = pConnection->IsAccepSocket(pSocket);
    if (pNode->isLocal)
        pConnNode->cSend += cData;
    else
        pConnNode->cRecvd += cData;
    pConnNode->add_child(pNode);
    return pNode;
}

LOG_NODE* Archive::importNode(DWORD size, FILE* fp, LOG_NODE* pParentNode, DWORD data_type)
{
    ATLASSERT(size > 0);
    LOG_NODE* pNode = (CONN_NODE*)m_pNodes->Add(size + sizeof(LOG_NODE), true);
    if (pNode) {
        if (1 != fread(((char*)pNode) + sizeof(LOG_NODE), size, 1, fp))
            pNode = nullptr;
    }
    if (pNode) {
        pNode->data_type = (LOG_TYPE)data_type;
        pParentNode->add_child(pNode);
    }
    return pNode;
}
