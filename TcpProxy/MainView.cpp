#include "stdafx.h"
#include "MainView.h"

CMainView::CMainView()
{

}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CMainView::OnCreate(LPCREATESTRUCT lpcs)
{
	return 0; // windows sets focus to first control
}

LRESULT CMainView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rcItem = { 0,0,0,0 }, rcEraze;
	GetClientRect(&rcEraze);
	HBRUSH brush = ::CreateSolidBrush(RGB(0,0,128));
	::FillRect((HDC)wParam, &rcEraze, brush);
	DeleteObject(brush);
	return 1; // handled
}

LRESULT CMainView::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CMainView::OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	return CDRF_DODEFAULT;
}

LRESULT CMainView::OnPositionChanging(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	WINDOWPOS* pWinPos = reinterpret_cast<WINDOWPOS*>(lParam);
	if (!(pWinPos->flags & SWP_NOSIZE))
	{
		int cx = pWinPos->cx, cy = pWinPos->cy;
		if (cx && cy)
		{
			//!!m_wndHorzSplitter.MoveWindow(0, 0, cx, cy, TRUE);
		}
	}
	return 1;
}
