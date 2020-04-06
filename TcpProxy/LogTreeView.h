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
	END_MSG_MAP()

	void Clear();
	void RefreshTree();
	void DrawSubItem(int iItem, int iSubItem, HDC hdc, RECT rcItem);

	void OnSize(UINT nType, CSize size);

	DWORD GetRecCount() { return m_recCount; }
	LOG_NODE* GetSelectedNode() { return m_pSelectedNode; }

private:
	LOG_NODE* getTreeNode(int iItem, int* pOffset = NULL);
	void GetNodetPos(HDC hdc, int offset, char* szText, int cbText, int& cxText, int& xStart, int& xEnd);
	void SetColumnLen(int len);

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

