#include "stdafx.h"
#include "MainFrame.h"
#include "aboutdlg.h"
#include "Settings.h"
#include "Proxy.h"

HWND  hwndMain;

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
    }

    return 0;
}

LRESULT CMainFrame::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    gSettings.SaveWindPos(m_hWnd);
    gProxy.Stop();
    bHandled = FALSE;
    return 0;
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
    gProxy.Start();
    ROUTE rote;
    rote.m_local_port = 5566;
    rote.m_remoute_port = 5567;
    rote.m_remoute_addr = "192.168.0.64";
    gProxy.AddRoute(rote);
    return 0;
}

LRESULT CMainFrame::OnStopProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    gProxy.Stop();
    return 0;
}

LRESULT CMainFrame::onShowMsg(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CHAR* buf = (CHAR*)wParam;
    MessageBox(buf, TEXT("Flow Trace Error"), MB_OK | MB_ICONEXCLAMATION);
    delete buf;
    return true;
}
