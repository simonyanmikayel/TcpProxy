#pragma once

#include "Archive.h"
#include "Helpers.h"

class CMainView;

class CLogTreeView : public CWindowImpl<CLogTreeView, CListViewCtrl>
{
public:
	CLogTreeView(CMainView* pView);
	~CLogTreeView();

	BEGIN_MSG_MAP(CLogTreeView)
		MSG_WM_SIZE(OnSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	END_MSG_MAP()

	void OnSize(UINT nType, CSize size);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnRButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void Clear();
	void RefreshTree();
	void DrawSubItem(int iItem, int iSubItem, HDC hdc, RECT rcItem);

	DWORD GetRecCount() { return m_recCount; }
	LOG_NODE* GetSelectedNode() { return m_pSelectedNode; }
	void ApplySettings();

private:
	LOG_NODE* getTreeNode(int iItem, int* pOffset = NULL);
	void GetNodetPos(HDC hdc, int offset, char* szText, int cbText, int& cxText, int& xStart, int& xEnd);
	void SetColumnLen(int len);
	LRESULT _OnKeyDown(WPARAM virt_key, bool bShiftPressed, bool bCtrlPressed);
	int ItemByPos(int yPos);
	int hitTest(LOG_NODE* pNode, int xPos, int offset);
	void CollapseExpandAll(LOG_NODE* pNode, bool expand);
	void SetSelectedNode(LOG_NODE* pNode);
	void EnsureItemVisible(int iItem);
	void ShowItem(DWORD i, bool scrollToMiddle);

	CMainView* m_pView;
	DWORD m_recCount;
	HIMAGELIST m_hTypeImageList;
	HIMAGELIST m_hStateImageList;
	int m_colWidth, m_rowHeight;
	bool m_Initialised;
	LOG_NODE* m_pSelectedNode;
	HPEN hDotPen;
	HDC m_hdc;
	CSize m_size;
};

