#include "stdafx.h"
#include "Archive.h"
#include "LogData.h"

DWORD LOG_NODE::nodeSize() {
    if (isRoot())
        return asRoot()->size();
    else if (isRouter())
        return asRouter()->size();
    else if (isConn())
        return asConn()->size();
    else if (isExchange())
        return asExchange()->size();
    else {
        ATLASSERT(FALSE);
        return sizeof(*this);
    }
}

void LOG_NODE::CalcLines()
{
    LOG_NODE* pNode = gArchive.getRootNode();
    int l = -1;
    //stdlog("CalcLines %d\n", GetTickCount());
    do
    {
        pNode->cExpanded = 0;
        pNode->pathExpanded = (!pNode->parent || (pNode->parent->pathExpanded && pNode->parent->expanded));
        if (pNode->parent)
            pNode->parent->cExpanded++;
        pNode->posInTree = ++l;
        //stdlog("index = %d e = %d l = %d %s\n", pNode->index, pNode->cExpanded, pNode->line, pNode->getTreeText());

        if (pNode->firstChild && pNode->expanded)
        {
            pNode = pNode->firstChild;
        }
        else if (pNode->nextSibling)
        {
            pNode = pNode->nextSibling;
        }
        else
        {
            while (pNode->parent)
            {
                pNode = pNode->parent;
                //stdlog("\t index = %d e = %d l = %d %s\n", pNode->index, pNode->cExpanded, pNode->line, pNode->getTreeText());
                if (pNode->parent && pNode->cExpanded)
                {
                    pNode->parent->cExpanded += pNode->cExpanded;
                    //stdlog("\t\t index = %d e = %d l = %d %s\n", pNode->parent->index, pNode->parent->cExpanded, pNode->parent->line, pNode->parent->getTreeText());
                }
                if (pNode == gArchive.getRootNode())
                    break;
                if (pNode->nextSibling)
                {
                    pNode = pNode->nextSibling;
                    break;
                }
            }
        }
    } while (gArchive.getRootNode() != pNode);
    //stdlog("CalcLines %d\n", GetTickCount());
}

int LOG_NODE::getTreeImage()
{
    if (asRoot())
    {
        return 0;//IDI_ICON_TREE_ROOT
    }
    else if (asRouter())
    {
        return 1;//IDI_ICON_TREE_ROUTE
    }
    else if (CONN_NODE* p = asConn())
    {
        if (p->closed)
            return 4;//IDI_ICON_TREE_CONN_CLOSED
        else if (p->opened)
            return 3;//IDI_ICON_TREE_CONN_CONNECTED
        else
            return 2;//IDI_ICON_TREE_CONN_INITIAL
    }
    else if (EXCHANGE_NODE* p = asExchange())
    {
        return p->isLocal ? 5 : 6;//IDI_ICON_TREE_RECV_LOCAL or IDI_ICON_TREE_RECV_REMOTE
    }
    else
    {
        ATLASSERT(FALSE);
        return 0;//IDI_ICON_TREE_ROOT
    }
}

const char* CONN_NODE::closeReason()
{
    const char* dueTo = "?";
    if (closed) {
        if (action == IO_ACTION::ACCEPT) {
            dueTo = "Client closed connection.";
        }
        else if (action == IO_ACTION::CONNECT) {
            dueTo = "Server rejeted connection.";
        }
        else if (action == IO_ACTION::RECV) {
            if (error_source == ERROR_SOURCE::CLIENT)
                dueTo = "Recieve failed. Connection closed by client.";
            else if (error_source == ERROR_SOURCE::SERVER)
                dueTo = "Recieve failed. Connection closed by server.";
            else
                dueTo = "Recieve failed. Connection closed by proxy.";
        }
        else if (action == IO_ACTION::SEND) {
            if (error_source == ERROR_SOURCE::CLIENT)
                dueTo = "Send failed. Connection closed by client.";
            else if (error_source == ERROR_SOURCE::SERVER)
                dueTo = "Send failed. Connection closed by server.";
            else
                dueTo = "Send failed. Connection closed by proxy.";
        }
        else if (action == IO_ACTION::PROXY_STOP) {
            dueTo = "proxy stopped.";
        }
    }
    return dueTo;
}

CHAR* LOG_NODE::getTreeText(int* cBuf)
{
    const int cMaxBuf = 1024;
    static CHAR pBuf[cMaxBuf + 1];
    int cb = 0;
    CHAR* ret = pBuf;
#ifdef _DEBUG
    //cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d %d %d]"), GetExpandCount(), line, lastChild ? lastChild->index : 0);
#endif
    if (nn)
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "%d - ", nn);
    }

    if (asRoot())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("   "));
    }
    else if (ROUTER_NODE* p = asRouter())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "%s (%d->%s:%d)",
            p->name(), p->local_port, p->remote_addr(), p->remote_port);
    }
    else if (CONN_NODE* p = asConn())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("%s (Total %d packages, sent %d bytes, received %d bytes) "), p->peername, p->childCount, p->cSend, p->cRecvd);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(created at %02d:%02d:%02d.%03d) "), p->initTime.wHour, p->initTime.wMinute, p->initTime.wSecond,p->initTime.wMilliseconds);
        if (p->closed)
        {
            const char* dueTo = p->closeReason();
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(clased at %02d:%02d:%02d.%03d) (%s) "),
                p->closeTime.wHour, p->closeTime.wMinute, p->closeTime.wSecond, p->closeTime.wMilliseconds, dueTo);
        }
    }
    else if (EXCHANGE_NODE* p = asExchange())
    {        
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("%s %d bytes "),
            p->isLocal ? "-> sent " : "<- received ", p->cData);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(at %02d:%02d:%02d.%03d) "),
            p->time.wHour, p->time.wMinute, p->time.wSecond, p->time.wMilliseconds);
    }
    else
    { 
        ATLASSERT(FALSE);
    }
    cb = std::min(cMaxBuf - 1, (int)cb);
    pBuf[cb] = 0;
    if (cBuf)
        *cBuf = cb;
    return ret;
}

void LOG_NODE::CollapseExpandAll(bool expand)
{
    //stdlog("CollapseExpandAll %d\n", GetTickCount());
    LOG_NODE* pNode = this;
    LOG_NODE* pNode0 = pNode;
    do
    {
        pNode->expanded = (pNode->lastChild && expand) ? 1 : 0;

        if (pNode->firstChild)
        {
            pNode = pNode->firstChild;
        }
        else if (pNode->nextSibling)
        {
            pNode = pNode->nextSibling;
        }
        else
        {
            while (pNode->parent)
            {
                pNode = pNode->parent;
                if (pNode == pNode0)
                    break;
                if (pNode->nextSibling)
                {
                    pNode = pNode->nextSibling;
                    break;
                }
            }
        }
    } while (pNode0 != pNode);
    //stdlog("CollapseExpandAll %d\n", GetTickCount());
    CalcLines();
}

void LOG_NODE::CollapseExpand(BOOL expand)
{
    expanded = expand;
    CalcLines();
}
