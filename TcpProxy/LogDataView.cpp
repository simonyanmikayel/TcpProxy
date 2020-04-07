#include "stdafx.h"
#include "MainView.h"
#include "Settings.h"
#include "Router.h"
#include "LogDataView.h"

CLogDataView::CLogDataView(CMainView* pView)
{
}

void CLogDataView::OnParentCreat()
{
    if (gSettings.routes.Get().size() == 0)
        SetWindowText("Select 'View'-'Route table' menu item to add a route.");
}

void CLogDataView::ApplySettings()
{
    SetFont(gSettings.GetFont());
}

void CLogDataView::Clear()
{
	SetWindowText("");
}

void CLogDataView::OnSelectionChanged(LOG_NODE* pNode)
{
    static const int cMaxBuf = 4 * Socket::bufSize;
    static char pBuf[cMaxBuf];
    int cb = 0;
    if (pNode->isRoot())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, ":)");
    }
    else if (pNode->isRouter())
    {
        ROUTER_NODE* This = (ROUTER_NODE*)this;
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, "%s ", This->name());
    }
    else if (pNode->isConn())
    {
        CONN_NODE* This = (CONN_NODE*)this;
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, "%s\nsent %d bytes, received %d bytes\n", This->peername, This->cSend, This->cRecvd);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, "created at %d:%d:%d.%d\n", This->initTime.wHour, This->initTime.wMinute, This->initTime.wSecond, This->initTime.wMilliseconds);
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
            cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, "clased at %d:%d:%d.%d due to %s\n",
                This->initTime.wHour, This->initTime.wMinute, This->initTime.wSecond, This->initTime.wMilliseconds, dueTo);
        }
    }
    else if (pNode->isRecv())
    {
        RECV_NODE* This = (RECV_NODE*)this;
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, "%s %d bytes\n\n",
            This->isLocal ? "-> sent " : "<- received ", This->cData);
    }
    else
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf, cMaxBuf, ":(");
    }
    cb = std::min(cMaxBuf - 1, (int)cb);
    pBuf[cb] = 0;
    SetWindowText(pBuf);
}