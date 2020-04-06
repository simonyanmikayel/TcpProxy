#pragma once

#include "Buffer.h"
#include "Router.h"

enum class LOG_TYPE { ROOT, ROUTE, CONN, RECV};

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

    bool isRoot() { return data_type == LOG_TYPE::ROOT; }
    bool isRoute() { return data_type == LOG_TYPE::ROUTE; }
    bool isConn() { return data_type == LOG_TYPE::CONN; }
    bool isRecv() { return data_type == LOG_TYPE::RECV; }

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
        LOG_NODE* p = pNode->parent;
        while (p)
        {
            p->childCount++;
            p = p->parent;
        }
    }

    void CalcLines();
    CHAR* getTreeText(int* cBuf = NULL, bool extened = true);
    int getTreeImage();
    int GetExpandCount() { return expanded ? cExpanded : 0; }
};

struct ROOT_NODE : LOG_NODE
{
};

struct ROUTER_NODE : LOG_NODE
{
    DWORD id;
};

struct CONN_NODE : LOG_NODE
{
    DWORD id;
};

struct RECV_NODE : LOG_NODE
{
    boolean isLocal;
};
