#pragma once

#include "stdafx.h"
#include "Helpers.h"
#include "resource.h"
#include "MainView.h"

class CMainFrame :
    public CFrameWindowImpl<CMainFrame>,
    public CUpdateUI<CMainFrame>,
    public CMessageFilter, public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
    CMainFrame();

    CMainView m_view;
    CCommandBarCtrl m_CmdBar;

    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual BOOL OnIdle();

    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_VIEW_STARTPROXY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_VIEW_STOPPROXY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_SHOW_NGS, onShowMsg)
        MESSAGE_HANDLER(WM_UPDATE_TREE, onUpdateTree)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_VIEW_STARTPROXY, OnStartProxy)
        COMMAND_ID_HANDLER(ID_VIEW_STOPPROXY, OnStopProxy)
        COMMAND_ID_HANDLER(ID_VIEW_PROXYSETTINGS, OnProxySettings)
        COMMAND_ID_HANDLER(ID_VIEW_CLEARLOG, OnClearLog)

        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
    LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnStartProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnStopProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnProxySettings(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnClearLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT onShowMsg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT onUpdateTree(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    void UpdateStatusBar();

private:
    void RefreshLog();
    void StartLogging();
    void StopLogging();
    void ClearLog();
};

extern CMainFrame* gMainFrame;
