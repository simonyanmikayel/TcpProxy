#pragma once

class CMainView : public CWindowImpl<CMainView>
{
public:
	DECLARE_WND_CLASS(NULL)

	CMainView();

    BOOL PreTranslateMessage(MSG* pMsg);

    BEGIN_MSG_MAP(CFlowTraceView)
        MSG_WM_CREATE(OnCreate)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, OnPositionChanging)

        NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
    END_MSG_MAP()

    LRESULT OnCreate(LPCREATESTRUCT lpcs);
    LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnPositionChanging(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
};

