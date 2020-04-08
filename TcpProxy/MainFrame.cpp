#include "stdafx.h"
#include "MainFrame.h"
#include "aboutdlg.h"
#include "Settings.h"
#include "Proxy.h"
#include "DlgRouteTable.h"
#include "DlgSettings.h"

HWND  hwndMain;
CMainFrame* gMainFrame;
#define	TIMER_DATA_REFRESH 43
#define	TIMER_DATA_REFRESH_INTERVAL	500

CMainFrame::CMainFrame()
{
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
    if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
        return TRUE;

    return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
    UIEnable(ID_VIEW_STARTPROXY, !gProxy.Running());
    UIEnable(ID_VIEW_STOPPROXY, gProxy.Running());
    UIUpdateToolBar();
    return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    hwndMain = m_hWnd;
    // create command bar window
    HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    // attach menu

    m_CmdBar.AttachMenu(GetMenu());
    // load command bar images
    BOOL b1 = m_CmdBar.LoadImages(IDR_MAINFRAME);
    // remove old menu
    SetMenu(NULL);

    CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
    AddSimpleReBarBand(hWndCmdBar);

    HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
    AddSimpleReBarBand(hWndToolBar, NULL, FALSE);
    SizeSimpleReBarBands();
    UIAddToolBar(hWndToolBar);

    CreateSimpleStatusBar();

    m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

    // register object for message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    return 0;
}

LRESULT CMainFrame::OnActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    static bool activated = false;
    if (!activated && wParam != WA_INACTIVE)
    {
        activated = true;
        gSettings.RestoreWindPos(m_hWnd);

        PostMessage(WM_COMMAND, ID_VIEW_STARTPROXY, 0);
        //PostMessage(WM_COMMAND, ID_VIEW_ROUTE_TABLSE, 0);
    }

    return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    gSettings.SaveWindPos(m_hWnd);
    StopLogging();    
    bHandled = FALSE;
    return 0;
}

LRESULT CMainFrame::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    // unregister message filtering and idle updates
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

    bHandled = FALSE;
    return 1;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
}

LRESULT CMainFrame::OnStartProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    StartLogging();
    return 0;
}

LRESULT CMainFrame::OnStopProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    StopLogging();
    return 0;
}

LRESULT CMainFrame::OnClearLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ClearLog();
    return 0;
}

LRESULT CMainFrame::OnRouteTable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DlgRouteTable dlg;
    dlg.DoModal();
    auto& v1 = dlg.routes;
    auto& v2 = gSettings.routes.Get();
    bool isEqual = true;
    if (v1.size() != v2.size())
        isEqual = false;
    if (isEqual)
    {
        for (int i = 0; i < v1.size(); i++)
        {
            if (!(v1[i] == v2[i]))
            {
                isEqual = false;
                break;
            }
        }
    }
    if (!isEqual)
    {
        gSettings.routes.Set(dlg.routes);
        StopLogging();
        StartLogging();
    }
    return 0;
}

LRESULT CMainFrame::OnProxySettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    DlgSettings dlg;
    if (IDOK == dlg.DoModal())
    {
        m_view.ApplySettings();
    }
    return 0;
}

LRESULT CMainFrame::onShowMsg(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CHAR* buf = (CHAR*)wParam;
    MessageBox(buf, TEXT("Flow Trace Error"), MB_OK | MB_ICONEXCLAMATION);
    delete buf;
    return true;
}

LRESULT CMainFrame::onUpdateTree(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    int nFirst = (int)wParam;
    int nLast = (int)lParam;
    if (nFirst > 0 && nLast > 0)
        m_view.m_wndTreeView.RedrawItems(nFirst, nLast);
    return true;
}

LRESULT CMainFrame::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (wParam == TIMER_DATA_REFRESH)
    {
        if (gProxy.Running())
        {
            RefreshLog();
        }
    }
    return 0;

}

void CMainFrame::RefreshLog()
{
    m_view.m_wndTreeView.RefreshTree();
}

void CMainFrame::StartLogging()
{
    gArchive.lock();
    gArchive.clearArchive();
    m_view.ClearLog();
    gProxy.Start(gSettings.routes.Get());
    SetTimer(TIMER_DATA_REFRESH, TIMER_DATA_REFRESH_INTERVAL);
    gArchive.unlock();
}

void CMainFrame::StopLogging()
{
    gProxy.Stop();
    KillTimer(TIMER_DATA_REFRESH);
}

void CMainFrame::ClearLog()
{
    gArchive.lock();
    gArchive.clearArchive();
    m_view.ClearLog();
    gProxy.ShowRoutes();
    gArchive.unlock();
}

void CMainFrame::UpdateStatusBar()
{
    static CHAR pBuf[1024];
    int cb = 0;
    cb += _sntprintf_s(pBuf + cb, _countof(pBuf) - cb, _countof(pBuf) - cb - 1, TEXT("Count: %s"), Helpers::str_format_int_grouped(gArchive.getNodeCount()));
    //cb += _sntprintf_s(pBuf + cb, _countof(pBuf) - cb, _countof(pBuf) - cb - 1, TEXT("   Memory: %s mb"), Helpers::str_format_int_grouped((LONG_PTR)(gArchive.UsedMemory() / 1000000)));
    ::PostMessage(m_hWndStatusBar, SB_SETTEXT, 0, (LPARAM)pBuf);
}