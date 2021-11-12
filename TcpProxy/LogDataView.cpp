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
    static const size_t cMaxBuf = 6 * Socket::bufSize;
    static char pBuf[cMaxBuf + 1];
    size_t cb = 0;
    if (pNode->asRoot())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, ":)\r\n");
        //char pp[] = { 0,1,2,3,4,5,6,7,8,9,'a','b','c','d','e','f','j','h','k','l' };
        //cb += Helpers::HexDump((BYTE*)(pBuf + cb), cMaxBuf - cb, (BYTE*)pp, sizeof(pp), 32);
    }
    else if (ROUTER_NODE* p = pNode->asRouter())
    {        
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "Name:        %s\r\nLocal port:  %d\r\nRemote port: %d\r\nRemote addr: %s",
            p->name(), p->local_port, p->remote_port, p->remote_addr()); 
    }
    else if (CONN_NODE* p = pNode->asConn())
    {        
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "Client IP: %s\r\nsent %d bytes\r\nreceived %d bytes\r\n", p->peername, p->cSend, p->cRecvd);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "created at %02d:%02d:%02d.%03d\r\n", p->initTime.wHour, p->initTime.wMinute, p->initTime.wSecond, p->initTime.wMilliseconds);
        if (p->opened)
        {
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "connected at %02d:%02d:%02d.%03d\r\n",
                p->connectTime.wHour, p->connectTime.wMinute, p->connectTime.wSecond, p->connectTime.wMilliseconds);
        }        
        if (p->closed)
        {
            const char* dueTo = p->closeReason();
            cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "closed at %02d:%02d:%02d.%03d\r\n%s\r\n",
                p->closeTime.wHour, p->closeTime.wMinute, p->closeTime.wSecond, p->closeTime.wMilliseconds, dueTo);
        }
    }
    else if (EXCHANGE_NODE* p = pNode->asExchange())
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, "%s %d bytes ",
            p->isLocal ? "-> send " : "<- received ", p->cData);
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, TEXT("(at %02d:%02d:%02d.%03d)\r\n\r\n"),
            p->time.wHour, p->time.wMinute, p->time.wSecond, p->time.wMilliseconds);
        cb += Helpers::HexDump((BYTE*)(pBuf + cb), cMaxBuf - cb, (BYTE*)p->data(), p->cData, 16);
    }
    else
    {
        cb += _sntprintf_s(pBuf + cb, cMaxBuf - cb, cMaxBuf - cb, ":(");
    }
    cb = std::min(cMaxBuf - 1, cb);
    pBuf[cb] = 0;
    SetWindowText(pBuf);
}