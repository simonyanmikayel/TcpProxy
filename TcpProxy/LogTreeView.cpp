#include "stdafx.h"
#include "resource.h"
#include "MainView.h"
#include "LogTreeView.h"

enum STATE_IMAGE { STATE_IMAGE_COLAPSED, STATE_IMAGE_EXPANDED, STATE_IMAGE_CHECKED, STATE_IMAGE_UNCHECKE };
static const int min_colWidth = 16;

CLogTreeView::CLogTreeView(CMainView* pView)
	: m_pView(pView)
	, m_recCount(0)
	, m_pSelectedNode(0)
	, m_rowHeight(1)
{
	m_hTypeImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0);

	ImageList_AddIcon(m_hTypeImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_TREE_ROOT))); //0
	ImageList_AddIcon(m_hTypeImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_TREE_ROUTE))); //1
	ImageList_AddIcon(m_hTypeImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_TREE_CONN))); //2
	ImageList_AddIcon(m_hTypeImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_TREE_RECV_LOCAL))); //3
	ImageList_AddIcon(m_hTypeImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_TREE_RECV_REMOTE))); //4

	m_colWidth = min_colWidth;
	m_Initialised = false;
	m_hdc = CreateCompatibleDC(NULL);// CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);

	LOGBRUSH LogBrush;
	LogBrush.lbColor = RGB(0, 0, 0);
	LogBrush.lbStyle = PS_SOLID;
	hDotPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &LogBrush, 0, NULL);

	m_hStateImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0);
	
	ImageList_AddIcon(m_hStateImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_NODE_COLAPSED))); //0 STATE_IMAGE_COLAPSED
	ImageList_AddIcon(m_hStateImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_NODE_EXPANDED))); //1 STATE_IMAGE_EXPANDED
	ImageList_AddIcon(m_hStateImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_NODE_CHECKED))); //2 STATE_IMAGE_CHECKED
	ImageList_AddIcon(m_hStateImageList, LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_ICON_NODE_UNCHECKE))); //3 STATE_IMAGE_UNCHECKE	
}

CLogTreeView::~CLogTreeView()
{
	DeleteDC(m_hdc);
}

void CLogTreeView::OnSize(UINT nType, CSize size)
{
	m_size = size;
	if (!m_Initialised)
	{
		m_Initialised = true;
		InsertColumn(0, "");
	}
}

void CLogTreeView::Clear()
{
	m_recCount = 0;
	m_pSelectedNode = NULL;
	SetItemCountEx(0, 0);
	m_colWidth = min_colWidth;
	SetColumnWidth(0, m_colWidth);
	Helpers::UpdateStatusBar();
}

void CLogTreeView::RefreshTree()
{
	DWORD newCount = gArchive.getNodeCount();
	if (newCount <= 1)
		return;//only root node

	if (newCount <= m_recCount)
	{
		ATLASSERT(newCount == m_recCount);
		return;
	}

	if (!m_Initialised)
		return;

	int prevExpanded = gArchive.getRootNode()->GetExpandCount();
	int firstAffected = -1;
	gArchive.getRootNode()->CalcLines();

	//add new logs
	for (DWORD i = m_recCount; i < newCount; i++)
	{
		LOG_NODE* p = gArchive.getNode(i);
		if (p && p->parent && p->parent->pathExpanded)
		{
			if (p->parent->GetExpandCount() > 0 && !p->parent->hasNodeBox)
			{
				p->parent->hasNodeBox = 1;
				if (firstAffected == -1 || firstAffected > p->parent->posInTree)
					firstAffected = p->parent->posInTree;
			}
			if (p->parent->expanded)
			{
				if (firstAffected == -1 || firstAffected > p->posInTree)
					firstAffected = p->posInTree;
			}
		}
	}

	if (m_recCount != 0)
	{
		if (prevExpanded != gArchive.getRootNode()->GetExpandCount())
		{
			SetItemCountEx(gArchive.getRootNode()->GetExpandCount() + 1, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);//LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL
		}
		if (firstAffected > -1)
		{
			RedrawItems(firstAffected, gArchive.getRootNode()->GetExpandCount());
		}
	}

	if (m_recCount == 0)
		SetItemCountEx(1, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);//LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL

	m_recCount = newCount;
	Helpers::UpdateStatusBar();
}

LOG_NODE* CLogTreeView::getTreeNode(int iItem, int* pOffset)
{
	int offset;
	LOG_NODE* pNode;
	bool did_again = false;

	//#ifdef _DEBUG
	//  if (iItem == 5)
	//    iItem = iItem;
	//#endif
	  //stdlog("getTreeNode-> %d\n", GetTickCount());

again:
	offset = 0;
	pNode = gArchive.getRootNode();
	ATLASSERT(gArchive.getRootNode()->expanded || iItem == 0);

	while (iItem != pNode->posInTree)
	{
		//stdlog("\tline %d offset = %d %s\n", pNode->line, offset, pNode->getTreeText());
		//if (pNode->nextSibling && (pNode->line + pNode->GetExpandCount()) < iItem)
		if (pNode->nextSibling && pNode->nextSibling->posInTree <= iItem)
		{
			while (pNode->nextChank && pNode->nextChank->posInTree <= iItem)
			{
				//stdlog("\t1 index = %d line %d offset = %d %s\n", pNode->index, pNode->line, offset, pNode->getTreeText());
				pNode = pNode->nextChank;
				//stdlog("\t\t1 index = %d line %d offset = %d %s\n", pNode->index, pNode->line, offset, pNode->getTreeText());
			}
			while (pNode->nextSibling && pNode->nextSibling->posInTree <= iItem)
			{
				//stdlog("\t2 index = %d line %d offset = %d %s\n", pNode->index, pNode->line, offset, pNode->getTreeText());
				pNode = pNode->nextSibling;
				//stdlog("\t\t2 index = %d line %d offset = %d %s\n", pNode->index, pNode->line, offset, pNode->getTreeText());
			}
		}
		else
		{
			//stdlog("\t\t\t3 index = %d line %d offset = %d %s\n", pNode->index, pNode->line, offset, pNode->getTreeText());

			if (!pNode->firstChild)
			{
				if (!did_again)
				{
					did_again = true;
					goto again;
				}
				else
				{
					//!!ATLASSERT(0);
					break;
				}
			}
			offset++;
			pNode = pNode->firstChild;
		}
		//stdlog("\tiItem %d offset = %d %s\n", iItem, offset, pNode->getTreeText());
	}
	//stdlog("getTreeNode<- %d\n", GetTickCount());
	//stdlog("\tiItem %d offset = %d %s\n", iItem, offset, pNode->getTreeText());
	if (pOffset)
		*pOffset = offset;
	return pNode;
}

void CLogTreeView::GetNodetPos(HDC hdc, int offset, char* szText, int cbText, int& cxText, int& xStart, int& xEnd)
{
	xStart = offset * ICON_OFFSET;
	xEnd = xStart;
	//next to button
	xEnd += ICON_OFFSET;
	//next to image
	xEnd += ICON_OFFSET;

	SIZE size;
	GetTextExtentPoint32(hdc, szText, cbText, &size);
	cxText = size.cx;
	xEnd += size.cx;
}

void CLogTreeView::SetColumnLen(int len)
{
	RECT rc;
	GetClientRect(&rc);
	len = max(rc.right, (long)len) + 16;
	if (m_colWidth < len)
	{
		//stdlog("m_colWidth %d len %d col %drc.right %d\n", m_colWidth, len, col, rc.right);
		m_colWidth = len;
		SetColumnWidth(0, len);
		Invalidate(FALSE);
	}
}

void CLogTreeView::DrawSubItem(int iItem, int iSubItem, HDC hdc, RECT rcItem)
{
	static const DWORD InfoTextColor = RGB(128, 128, 128);
	static const DWORD SelectionBkColor = RGB(64, 64, 192);

	if (iSubItem != 0)
	{
		ATLASSERT(FALSE);
		return;
	}
	m_rowHeight = rcItem.bottom - rcItem.top;
	int offset;
	LOG_NODE* pNode = getTreeNode(iItem, &offset);
	if (!pNode)
	{
		ATLASSERT(FALSE);
		return;
	}
	//POINT  point = { rcItem.left + offset * ICON_OFFSET, rcItem.top };

	//stdlog("DrawSubItem: iItem = %d\n", iItem);
	//stdlog("offset = %d rcItem l = %d r = %d t = %d b = %d\n", offset, rcItem.left, rcItem.right, rcItem.top, rcItem.bottom);
	int left = rcItem.left + offset * ICON_OFFSET;
	int top = rcItem.top;

	//HBRUSH hbr = ::CreateSolidBrush(RGB(255,255,255));
	//::FillRect(hdc, &rcItem, hbr);
	//::DeleteObject(hbr);

	int yMiddle = top + (rcItem.bottom - top) / 2;
	int yIconTop = yMiddle - ICON_LEN / 2;

	::SelectObject(hdc, hDotPen);
	MoveToEx(hdc, left + 1 + ICON_LEN / 2, yMiddle, 0);
	LineTo(hdc, left + ICON_OFFSET, yMiddle);

	if (pNode->lastChild)
	{
		ImageList_Draw(m_hStateImageList, pNode->expanded ? STATE_IMAGE_EXPANDED : STATE_IMAGE_COLAPSED, hdc, left, yIconTop, ILD_NORMAL);
	}
	//next to button
	left += ICON_OFFSET;

	ImageList_Draw(m_hTypeImageList, pNode->getTreeImage(), hdc, left, yIconTop, ILD_NORMAL);
	//next to image
	left += ICON_OFFSET;

	int cbText;

	char* szText = pNode->getTreeText(&cbText);
	int cxText, xStart, xEnd;
	GetNodetPos(hdc, offset, szText, cbText, cxText, xStart, xEnd);
	ATLASSERT(xEnd + rcItem.left == left + cxText);

	SetColumnLen(xEnd);

	COLORREF old_textColor = ::GetTextColor(hdc);
	COLORREF old_bkColor = ::GetBkColor(hdc);
	int old_bkMode = ::GetBkMode(hdc);


	RECT rcFrame = rcItem;
	rcFrame.left = left - 2;
	rcFrame.right = rcFrame.left + cxText + 4;
	::SetBkMode(hdc, OPAQUE);

	::SetTextColor(hdc, InfoTextColor);
	TextOut(hdc, left, top, szText, cbText);

	if (m_pSelectedNode == pNode)
	{
		if (GetFocus() == m_hWnd)
		{
			CBrush brush2;
			brush2.CreateSolidBrush(SelectionBkColor);
			FrameRect(hdc, &rcFrame, brush2);
		}
		else
		{
			DrawFocusRect(hdc, &rcFrame);
		}
		//::SetTextColor(hdc, gSettings.SelectionTxtColor());
	}

	int i = 0, x = left - ICON_OFFSET - ICON_LEN / 2 - 5;
	for (LOG_NODE* p = pNode; p; i++, x = x - ICON_OFFSET)
	{
		bool drawUp = false, drawDawn = false, isParent = p != pNode;
		drawUp = drawUp || (!isParent && (p->prevSibling || p->parent));
		drawUp = drawUp || (isParent && p->nextSibling);
		drawDawn = drawDawn || (!isParent && p->nextSibling);
		drawDawn = drawDawn || (isParent && p->nextSibling);

		if (drawUp)
		{
			MoveToEx(hdc, x, top, 0);
			LineTo(hdc, x, isParent ? yMiddle + 1 : (p->lastChild ? yMiddle - 5 : (yMiddle + 1)));
		}
		if (drawDawn)
		{
			MoveToEx(hdc, x, rcItem.bottom, 0);
			LineTo(hdc, x, isParent ? yMiddle : (p->lastChild ? yMiddle + 6 : (yMiddle)));
		}
		p = p->parent;
	}

	::SetTextColor(hdc, old_textColor);
	::SetBkColor(hdc, old_bkColor);
	::SetBkMode(hdc, old_bkMode);
}
