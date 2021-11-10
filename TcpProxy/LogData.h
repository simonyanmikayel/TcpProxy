#pragma once

#include "Buffer.h"
#include "Router.h"

enum class LOG_TYPE { ROOT, ROUTER, CONN, RECV};
struct ROOT_NODE;
struct ROUTER_NODE;
struct CONN_NODE;
struct RECV_NODE;

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
    BYTE bookmark;
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
    RECV_NODE* asRecv() { return data_type == LOG_TYPE::RECV ? (RECV_NODE*)this : nullptr; }

    //bool isRoot() { return data_type == LOG_TYPE::ROOT; }
    //bool isRouter() { return data_type == LOG_TYPE::ROUTER; }
    //bool isConn() { return data_type == LOG_TYPE::CONN; }
    //bool isRecv() { return data_type == LOG_TYPE::RECV; }

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
        while (p)
        {
            p->childCount++;
            p = p->parent;
        }
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
};

struct ROUTER_NODE : LOG_NODE
{
    DWORD id;
    DWORD local_port;
    DWORD remote_port;
    WORD cb_name;
    WORD cb_remote_addr;
    char* name() { return (char*)(this) + sizeof(*this); }
    char* remote_addr() { return name() + cb_name + 1; }
};

struct CONN_NODE : LOG_NODE
{
    DWORD id;
    DWORD cSend;
    DWORD cRecvd;
    BYTE  opened;
    BYTE  closed;
    char peername[32];
    IO_ACTION action;
    SYSTEMTIME initTime;
    SYSTEMTIME connectTime;
    SYSTEMTIME closeTime;
};

struct RECV_NODE : LOG_NODE
{
    SYSTEMTIME time;
    boolean isLocal;
    DWORD cData;
    char* data() { return (char*)(this) + sizeof(*this); }
};
