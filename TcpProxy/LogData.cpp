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
    else if (isRoute())
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
    int NN = getNN();
#ifdef _DEBUG
    //cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, TEXT("[%d %d %d]"), GetExpandCount(), line, lastChild ? lastChild->index : 0);
#endif
    if (this == gArchive.getRootNode())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, TEXT("..."));
    }
    else if (isApp())
    {
        APP_NODE* This = (APP_NODE*)this;
        cb = This->cb_app_name;
        memcpy(pBuf, This->appName, cb);
        pBuf[cb] = 0;
        if (This->lost)
        {
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (Lost: %d)"), This->lost);
        }
        if (gSettings.GetShowAppIp() && This->ip_address[0])
        {
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (%s)"), This->ip_address);
        }
        if (gSettings.GetColPID())
        {
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d]"), This->pid);
        }
    }
    else if (isThread())
    {
        if (gSettings.GetColPID() && gSettings.GetColTID())
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d-%d-%d]"), getThreadNN(), getPid(), getTid());
        else if (gSettings.GetColPID())
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d-%d]"), getThreadNN(), getPid());
        else if (gSettings.GetColTID())
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d-%d]"), getThreadNN(), getTid());
        else
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d]"), getThreadNN());
        if (gSettings.GetShowChildCount())
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d]"), childCount);

    }
    else if (isFlow())
    {
        FLOW_NODE* This = (FLOW_NODE*)this;
        int cb_fn_name = This->cb_fn_name;
        char* name = This->shortFnName();
        memcpy(pBuf + cb, name, This->cb_fn_name - This->cb_short_fn_name_offset);
        cb += This->cb_fn_name - This->cb_short_fn_name_offset;
        pBuf[cb] = 0;
        if (extened)
        {
            if (gSettings.GetColNN() && NN)
                cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (%d)"), NN); //gArchive.index(this) NN
            if (gSettings.GetShowElapsedTime() && This->getPeer())
            {
                _int64 sec1 = This->getTimeSec();
                _int64 msec1 = This->getTimeMSec();
                _int64 sec2 = (This->getPeer())->getTimeSec();
                _int64 msec2 = (This->getPeer())->getTimeMSec();
                cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (%lldms)"), (sec2 - sec1) * 1000 + (msec2 - msec1));
            }
            if (gSettings.GetColCallAddr())
            {
                DWORD p = This->call_site;
                cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (%X)"), p);
            }
            if (gSettings.GetFnCallLine())
            {
                ADDR_INFO* p_call_addr_info = This->getCallInfo(false);
                if (p_call_addr_info != 0)
                    cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT(" (%d)"), p_call_addr_info->line);
            }
            if (gSettings.GetShowChildCount())
            {
                cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("[%d]"), childCount);
            }
        }
        pBuf[cb] = 0;
    }
    else
    {
        ATLASSERT(FALSE);
    }
    if (cb > cMaxBuf)
        cb = cMaxBuf;
    pBuf[cb] = 0;
    if (cBuf)
        *cBuf = cb;
    return ret;
}