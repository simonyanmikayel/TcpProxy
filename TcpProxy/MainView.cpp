#include "stdafx.h"
#include "Settings.h"
#include "MainView.h"

CMainView::CMainView()
	: m_wndTreeView(this)
	, m_wndDataView(this)
{
}

BOOL CMainView::PreTranslateMessage(MSG* pMsg)
{
	pMsg;
	return FALSE;
}

LRESULT CMainView::OnCreate(LPCREATESTRUCT lpcs)
{
	ModifyStyle(0, WS_CLIPCHILDREN);
	ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_TRANSPARENT);

	m_wndVertSplitter.m_bVertical = true;
	m_wndVertSplitter.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, SPLIT_PROPORTIONAL);

	m_wndTreeView.Create(m_wndVertSplitter, rcDefault, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
		LVS_REPORT | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_OWNERDATA | LVS_NOCOLUMNHEADER,
		LVS_EX_FULLROWSELECT);
	m_wndDataView.Create(m_wndVertSplitter, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		WS_VSCROLL | WS_HSCROLL |
		ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL | ES_WANTRETURN | ES_MULTILINE, 0);

	m_wndVertSplitter.SetSplitterPanes(m_wndTreeView, m_wndDataView);
	m_wndVertSplitter.SetSplitterPosPct(std::max(10, std::min(90, gSettings.vertSplitterPos.Get())), false);
	m_wndVertSplitter.m_bFullDrag = false;

	OnParentCreat();
	ApplySettings();
	return 0; // windows sets focus to first control
}

void CMainView::OnParentCreat()
{
	m_wndDataView.OnParentCreat();
}

LRESULT CMainView::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	gSettings.vertSplitterPos.Set(m_wndVertSplitter.GetSplitterPosPct());
	bHandled = FALSE;
	return 0;
}

LRESULT CMainView::OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//RECT rcItem = { 0,0,0,0 }, rcEraze;
	//GetClientRect(&rcEraze);
	//HBRUSH brush = ::CreateSolidBrush(RGB(128,0,0));
	//::FillRect((HDC)wParam, &rcEraze, brush);
	//DeleteObject(brush);
	return 1; // handled
}

LRESULT CMainView::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}

LRESULT CMainView::OnCustomDraw(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
	if (pnmh->hwndFrom == m_wndTreeView)
	{
		LPNMLVCUSTOMDRAW pNMLVCD = (LPNMLVCUSTOMDRAW)pnmh;
		switch (pNMLVCD->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;          // ask for subitem notifications.
		case CDDS_ITEMPREPAINT:
			return CDRF_NOTIFYSUBITEMDRAW;
		case CDDS_ITEMPREPAINT | CDDS_SUBITEM: // recd when CDRF_NOTIFYSUBITEMDRAW is returned in
		{                                    // response to CDDS_ITEMPREPAINT.
			m_wndTreeView.DrawSubItem((int)(pNMLVCD->nmcd.dwItemSpec), pNMLVCD->iSubItem, pNMLVCD->nmcd.hdc, pNMLVCD->nmcd.rc);
			return CDRF_SKIPDEFAULT;
		}
		break;
		}
		return CDRF_DODEFAULT;
	}
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
			m_wndVertSplitter.MoveWindow(0, 0, cx, cy, TRUE);
		}
	}
	return 1;
}

void CMainView::ClearLog()
{
	m_wndTreeView.Clear();
	m_wndDataView.Clear();
}

void CMainView::ApplySettings()
{
	m_wndTreeView.ApplySettings();
	m_wndDataView.ApplySettings();
}
void CMainView::OnSelectionChanged(LOG_NODE* pNode)
{
	m_wndDataView.OnSelectionChanged(pNode);
}

