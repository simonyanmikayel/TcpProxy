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
    if (isRoot())
    {
        return 0;//IDI_ICON_TREE_ROOT
    }
    else if (isRouter())
    {
        return 1;//IDI_ICON_TREE_ROUTE
    }
    else if (isConn())
    {
        return 2;//IDI_ICON_TREE_CONN
    }
    else if (isRecv())
    {
        RECV_NODE* This = (RECV_NODE*)this;
        return This->isLocal ? 3 : 4;//IDI_ICON_TREE_RECV_LOCAL or IDI_ICON_TREE_RECV_REMOTE
    }
    else
    {
        ATLASSERT(FALSE);
        return 0;//IDI_ICON_TREE_ROOT
    }
}

CHAR* LOG_NODE::getTreeText(int* cBuf, bool extened)
{
    const int cMaxBuf = 255;
    static CHAR pBuf[cMaxBuf + 1];
    int cb = 0;
    CHAR* ret = pBuf;
#ifdef _DEBUG
    //cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, TEXT("[%d %d %d]"), GetExpandCount(), line, lastChild ? lastChild->index : 0);
#endif
    if (this == gArchive.getRootNode())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, TEXT("..."));
    }
    else if (isRouter())
    {
        ROUTER_NODE* This = (ROUTER_NODE*)this;
        cb = std::min(cMaxBuf - 1, (int)This->cb_name);
        memcpy(pBuf, This->name(), cb);
        pBuf[cb] = 0;
    }
    else if (isConn())
    {
        //TODO - show pear addr
        CONN_NODE* This = (CONN_NODE*)this;
        cb = std::min(cMaxBuf - 1, 4);
        memcpy(pBuf, "TODO", cb);
        pBuf[cb] = 0;
    }
    else if (isRecv())
    {
        //TODO - show IO info
        RECV_NODE* This = (RECV_NODE*)this;
        cb = std::min(cMaxBuf - 1, 4);
        memcpy(pBuf, This->isLocal ? "IO->" : "IO<-" , cb);
        pBuf[cb] = 0;
    }
    else
    {
        ATLASSERT(FALSE);
        pBuf[cb] = 0;
    }
    if (cBuf)
        *cBuf = cb;
    return ret;
}