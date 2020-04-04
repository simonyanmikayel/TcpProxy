#pragma once

#define WM_SHOW_NGS                    WM_USER + 1003

namespace Helpers
{
	void ErrMessageBox(CHAR* lpFormat, ...);
	void SysErrMessageBox(CHAR* lpFormat, ...);
};
