#include "stdafx.h"
#include "MainView.h"
#include "MainViewRightPane.h"

CMainViewRightPane::CMainViewRightPane(CMainView* pView)
{

}

LRESULT CMainViewRightPane::OnCreate(LPCREATESTRUCT lpcs)
{
	ModifyStyle(0, WS_CLIPCHILDREN);
	ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_TRANSPARENT);


	return 0; // windows sets focus to first control
}

void CMainViewRightPane::ApplySettings()
{

}

LRESULT CMainViewRightPane::OnPositionChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetChiledPos();
	return 1;
}

void CMainViewRightPane::SetChiledPos()
{
	WINDOWPLACEMENT wpl = { sizeof(WINDOWPLACEMENT) };
	if (::GetWindowPlacement(m_hWnd, &wpl)) {
	}
}

LRESULT CMainViewRightPane::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT rcItem = { 0,0,0,0 }, rcEraze;
	GetClientRect(&rcEraze);
	HBRUSH brush = ::CreateSolidBrush(RGB(0,0,192));
	::FillRect((HDC)wParam, &rcEraze, brush);
	DeleteObject(brush);
	return 1; // handled
}


LRESULT CMainViewRightPane::OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	return CDRF_DODEFAULT;
}

void CMainViewRightPane::Clear()
{
	//TODO
}

