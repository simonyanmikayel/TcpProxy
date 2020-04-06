#pragma once

#define WM_SHOW_NGS                    WM_USER + 1003

static const int ICON_LEN = 16;
static const int ICON_OFFSET = 16 + 4;

namespace Helpers
{
	void ErrMessageBox(CHAR* lpFormat, ...);
	void SysErrMessageBox(CHAR* lpFormat, ...);
	void UpdateStatusBar();
	char* int2char(int i);
};
