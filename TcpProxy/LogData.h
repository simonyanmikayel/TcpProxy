#pragma once

#include "Buffer.h"
#include "Router.h"

enum class LOG_TYPE { ROOT, ROUTER, CONN, EXCHANGE};
struct ROOT_NODE;
struct ROUTER_NODE;
struct CONN_NODE;
struct EXCHANGE_NODE;

//#pragma pack(push,1)

struct LOG_NODE
{
    LOG_NODE* parent;
    LOG_NODE* firstChild;
    LOG_NODE* lastChild;
    LOG_NODE* prevSibling;
    LOG_NODE* nextSibling;
    struct {
        WORD hasNewLine : 1;
        WORD hiden : 1;
        WORD selected : 1;
        WORD expanded : 1;
        WORD hasNodeBox : 1;
        WORD pathExpanded : 1;
    };
    BYTE nextChankCounter;
    int cExpanded;
    int posInTree;
    int lineSearchPos;
    BYTE log_flags;
    LOG_NODE* nextChankMarker;
    LOG_NODE* nextChank;
    LOG_TYPE data_type;
    DWORD childCount;
    DWORD nn;

    ROOT_NODE* asRoot() { return data_type == LOG_TYPE::ROOT ? (ROOT_NODE*)this : nullptr; }
    ROUTER_NODE* asRouter() { return data_type == LOG_TYPE::ROUTER ? (ROUTER_NODE*)this : nullptr; }
    CONN_NODE* asConn() { return data_type == LOG_TYPE::CONN ? (CONN_NODE*)this : nullptr; }
    EXCHANGE_NODE* asExchange() { return data_type == LOG_TYPE::EXCHANGE ? (EXCHANGE_NODE*)this : nullptr; }
    DWORD nodeSize();

    bool isRoot() { return data_type == LOG_TYPE::ROOT; }
    bool isRouter() { return data_type == LOG_TYPE::ROUTER; }
    bool isConn() { return data_type == LOG_TYPE::CONN; }
    bool isExchange() { return data_type == LOG_TYPE::EXCHANGE; }

    void add_child(LOG_NODE* pNode)
    {
        if (!firstChild)
            firstChild = pNode;
        if (lastChild)
            lastChild->nextSibling = pNode;

        nextChankCounter++;
        if (nextChankCounter == 255) // 255 is max number of nextChankCounter. then it will start from 0 
        {
            if (!nextChankMarker)
                firstChild->nextChank = pNode;
            else
                nextChankMarker->nextChank = pNode;
            nextChankMarker = pNode;
        }
        pNode->prevSibling = lastChild;
        pNode->parent = this;
        lastChild = pNode;
        LOG_NODE* p = this;
        childCount++;
        pNode->nn = this->childCount;
    }

    void CalcLines();
    CHAR* getTreeText(int* cBuf = NULL);
    int getTreeImage();
    int GetExpandCount() { return expanded ? cExpanded : 0; }
    void CollapseExpandAll(bool expand);
    void CollapseExpand(BOOL expand);
    int GetPosInTree() { return posInTree; }
};

struct ROOT_NODE : LOG_NODE
{
    DWORD size() { return sizeof(*this); }
};

struct ROUTER_NODE : LOG_NODE
{
    DWORD id;
    DWORD local_port;
    DWORD remote_port;
    WORD cb_name;
    WORD cb_remote_addr;
    DWORD cSendCount;
    DWORD cSendSize;
    DWORD cRecvdCount;
    DWORD cRecvdSize;
    char* name() { return (char*)(this) + sizeof(*this); }
    char* remote_addr() { return name() + cb_name + 1; }
    DWORD size() { return sizeof(*this) + cb_name + 1 + cb_remote_addr + 1; }
};

struct CONN_NODE : LOG_NODE
{
    DWORD id;
    DWORD cSendCount;
    DWORD cSendSize;
    DWORD cRecvdCount;
    DWORD cRecvdSize;
    BYTE  opened;
    BYTE  closed;
    char peername[32];
    IO_ACTION action;
    ERROR_SOURCE error_source;
    SYSTEMTIME initTime;
    SYSTEMTIME connectTime;
    SYSTEMTIME closeTime;
    const char* closeReason();
    DWORD size() { return sizeof(*this); }
};

struct EXCHANGE_NODE : LOG_NODE
{
    SYSTEMTIME time;
    boolean isLocal;
    DWORD cData;
    char* data() { return (char*)(this) + sizeof(*this); }
    DWORD size() { return sizeof(*this) + cData; }
};

//#pragma pack(pop)