#include "StdAfx.h"
#include "Settings.h"
#include "resource.h"

CSettings gSettings;

// registry keys and values
LPCTSTR STR_APP_REG_KEY = _T("Software\\TermianlTools\\TcpProxy");
LPCTSTR STR_APP_REG_VAL_WINDOWPOS = _T("WindowPos");

CSettings::CSettings() :
	CRegKeyExt(STR_APP_REG_KEY)
{

}

CSettings::~CSettings()
{
}

void CSettings::RestoreWindPos(HWND hWnd)
{
	WINDOWPLACEMENT wpl;
	if (Read(STR_APP_REG_VAL_WINDOWPOS, &wpl, sizeof(wpl))) {
		RECT rcWnd = wpl.rcNormalPosition;
		int cx, cy, x, y;
		cx = rcWnd.right - rcWnd.left;
		cy = rcWnd.bottom - rcWnd.top;
		x = rcWnd.left;
		y = rcWnd.top;

		// Get the monitor info
		MONITORINFO monInfo;
		HMONITOR hMonitor = ::MonitorFromPoint(CPoint(x, y), MONITOR_DEFAULTTONEAREST);
		monInfo.cbSize = sizeof(MONITORINFO);
		if (::GetMonitorInfo(hMonitor, &monInfo))
		{
			// Adjust for work area
			x += monInfo.rcWork.left - monInfo.rcMonitor.left;
			y += monInfo.rcWork.top - monInfo.rcMonitor.top;
			// Ensure top left point is on screen
			if (CRect(monInfo.rcWork).PtInRect(CPoint(x, y)) == FALSE)
			{
				x = monInfo.rcWork.left;
				y = monInfo.rcWork.top;
			}
		}
		else
		{
			RECT rcScreen;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

			cx = min(rcScreen.right, (LONG)cx);
			cy = min(rcScreen.bottom, (LONG)cy);
			x = max(0L, min((LONG)x, rcScreen.right - cx));
			y = max(0L, min((LONG)y, rcScreen.bottom - cy));
		}

		::SetWindowPos(hWnd, 0, x, y, cx, cy, SWP_NOZORDER);

		if (wpl.flags & WPF_RESTORETOMAXIMIZED)
		{
			//::ShowWindow(hWnd, SW_SHOWMAXIMIZED);
			::PostMessage(hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}
	}
}

void CSettings::SaveWindPos(HWND hWnd)
{
	WINDOWPLACEMENT wpl = { sizeof(WINDOWPLACEMENT) };
	if (::GetWindowPlacement(hWnd, &wpl))
		Write(STR_APP_REG_VAL_WINDOWPOS, &wpl, sizeof(wpl));
}
