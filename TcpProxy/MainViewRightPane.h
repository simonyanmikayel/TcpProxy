#pragma once

class CMainView;
class CMainViewRightPane : public CWindowImpl<CMainViewRightPane>
{
public:
	DECLARE_WND_CLASS(NULL)

	CMainViewRightPane(CMainView* pView);

	BEGIN_MSG_MAP(CLogListFrame)
		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnPositionChanged)
		NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	END_MSG_MAP()

	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPositionChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);
	void SetChiledPos();
	void Clear();
};

