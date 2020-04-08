#include "stdafx.h"
#include "Archive.h"
#include "LogData.h"

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
    else if (asConn())
    {
        CONN_NODE* This = (CONN_NODE*)this;
        if (This->closed)
            return 4;//IDI_ICON_TREE_CONN_CLOSED
        else if (This->opened)
            return 3;//IDI_ICON_TREE_CONN_CONNECTED
        else
            return 2;//IDI_ICON_TREE_CONN_INITIAL
    }
    else if (asRecv())
    {
        RECV_NODE* This = (RECV_NODE*)this;
        return This->isLocal ? 5 : 6;//IDI_ICON_TREE_RECV_LOCAL or IDI_ICON_TREE_RECV_REMOTE
    }
    else
    {
        ATLASSERT(FALSE);
        return 0;//IDI_ICON_TREE_ROOT
    }
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
    if (asRoot())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("   "));
    }
    else if (ROUTER_NODE* This = asRouter())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("%s "), This->name());
    }
    else if (CONN_NODE* This = asConn())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("%s (sent %d bytes, received %d bytes) "), This->peername, This->cSend, This->cRecvd);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(created at %d:%d:%d.%d) "), This->initTime.wHour, This->initTime.wMinute, This->initTime.wSecond,This->initTime.wMilliseconds);
        if (This->closed)
        {
            char* dueTo = "?";
            if (This->action == IO_ACTION::ACCEPT)
                dueTo = "client closed connection";
            else if (This->action == IO_ACTION::CONNECT)
                dueTo = "server rejeted connection";
            else if (This->action == IO_ACTION::RECV || This->action == IO_ACTION::SEND)
                dueTo = "connection closed";
            else if (This->action == IO_ACTION::RECV || This->action == IO_ACTION::PROXY_STOP)
                dueTo = "proxy stopped";
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(clased at %d:%d:%d.%d) (%s) "),
                This->initTime.wHour, This->initTime.wMinute, This->initTime.wSecond, This->initTime.wMilliseconds, dueTo);
        }
    }
    else if (RECV_NODE* This = asRecv())
    {        
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("%s %d bytes "),
            This->isLocal ? "-> sent " : "<- received ", This->cData);
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
