#pragma once

class CMainView;
class CLogDataView : public CWindowImpl<CLogDataView, CEdit> //CEdit CRichEditCtrl
{
public:
	DECLARE_WND_CLASS(NULL)

	CLogDataView(CMainView* pView);

	BEGIN_MSG_MAP(CLogListFrame)
	END_MSG_MAP()

	void Clear();
	void ApplySettings();
	void OnSelectionChanged(LOG_NODE* pNode);
	void OnParentCreat();
};

