#pragma once

class CMainView;
class CLogDataView : public CWindowImpl<CLogDataView, CRichEditCtrl>
{
public:
	DECLARE_WND_CLASS(NULL)

	CLogDataView(CMainView* pView);

	BEGIN_MSG_MAP(CLogListFrame)
		MSG_WM_CREATE(OnCreate)
		MESSAGE_HANDLER(WM_WINDOWPOSCHANGED, OnPositionChanged)
	END_MSG_MAP()

	LRESULT OnCreate(LPCREATESTRUCT lpcs);
	LRESULT OnPositionChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	void SetChiledPos();
	void Clear();
	void ApplySettings();
};

