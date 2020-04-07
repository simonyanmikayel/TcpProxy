#include "stdafx.h"
#include "MainView.h"
#include "Settings.h"
#include "LogDataView.h"

CLogDataView::CLogDataView(CMainView* pView)
{

}

LRESULT CLogDataView::OnCreate(LPCREATESTRUCT lpcs)
{
	return 0; // windows sets focus to first control
}

void CLogDataView::ApplySettings()
{
	SetFont(gSettings.GetFont());
}

LRESULT CLogDataView::OnPositionChanged(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetChiledPos();
	return 1;
}

void CLogDataView::SetChiledPos()
{
	WINDOWPLACEMENT wpl = { sizeof(WINDOWPLACEMENT) };
	if (::GetWindowPlacement(m_hWnd, &wpl)) 
	{
	}
}

void CLogDataView::Clear()
{
	//TODO
}

