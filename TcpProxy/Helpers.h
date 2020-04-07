#pragma once

extern HWND hwndMain;

#define WM_SHOW_NGS                    WM_USER + 1003
#define WM_UPDATE_TREE                 WM_USER + 1004

static const int ICON_LEN = 16;
static const int ICON_OFFSET = 16 + 4;

namespace Helpers
{
	void ErrMessageBox(CHAR* lpFormat, ...);
	void SysErrMessageBox(CHAR* lpFormat, ...);
	void UpdateStatusBar();
	char* int2char(int i);
	void AddMenu(HMENU hMenu, int& cMenu, int ID_MENU, LPCTCH str, bool disable = false);
	CHAR* str_format_int_grouped(__int64 num);
};
