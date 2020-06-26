#include "StdAfx.h"
#include "Settings.h"
#include "Helpers.h"
#include "resource.h"

CSettings gSettings;

// registry keys and values
LPCTSTR STR_APP_REG_KEY = _T("Software\\TermianlTools\\TcpProxy");
LPCTSTR STR_APP_REG_VAL_WINDOWPOS = _T("WindowPos");

INT_VALUE::INT_VALUE(LPCTSTR sz, int defVal) : szRegKey(sz)
{
	gSettings.ReadINT(szRegKey, val, defVal);
}

void INT_VALUE::Set(int val)
{
	this->val = val;
	gSettings.WriteINT(szRegKey, val);
}

STR_VALUE::STR_VALUE(LPCTSTR sz, const char* defVal) : szRegKey(sz)
{
	DWORD cbData, dwDataTye;
	gSettings.CheckData(szRegKey, &cbData, &dwDataTye);
	if (dwDataTye == REG_SZ && cbData > 0 && cbData < MAX_DATA_SIZE)
	{
		std::string str;
		str.resize(cbData);
		if (gSettings.ReadSTR(szRegKey, (char*)str.c_str(), (int)cbData))
		{
			val = str;
		}
	}
	if (val.empty() && defVal)
	{
		val = defVal;
	}
}

void STR_VALUE::Set(char* val)
{
	this->val = val;
	gSettings.WriteSTR(szRegKey, val);
}

template<typename T> 
ARR_VALUE<T>::ARR_VALUE(LPCTSTR sz) : szRegKey(sz)
{
	DWORD cbData, dwDataTye;
	gSettings.CheckData(szRegKey, &cbData, &dwDataTye);
	if (dwDataTye == REG_SZ && cbData > 0 && cbData < MAX_DATA_SIZE)
	{
		try 
		{
			std::string str;
			str.resize(cbData);
			if (gSettings.ReadSTR(szRegKey, (char*)str.c_str(), (int)cbData))
			{
				//			str = "1\n2\n3\n4\n5\n";
							//Helpers::MaskWhiteSpace(str);
				std::stringstream s(str);

				std::string strCount;
				getline(s, strCount);
				int count = std::stoi(strCount);
				//s.setf(std::ios::skipws, 0);
				if (count > 0 && count < MAX_ARRAY_SIZE)
				{
					val.clear();
					val.reserve(count);
					for (int i = 0; i < count; i++)
					{
						T t;
						t.deserialize(s);
						val.push_back(t);
					}
				}
			}
		}
		catch (...) 
		{
			val.clear();
		}
	}
}

static const int DefFontSize = 12;
static const CHAR* DEF_FONT_NAME = _T("Consolas"); //Courier New //Consolas //Inconsolata

CSettings::CSettings() 
	: CRegKeyExt(STR_APP_REG_KEY)
	, vertSplitterPos(_T("vertSplitterPos"), 50)
	, routes(_T("routes"))
	, fontSize(_T("fontSize"), DefFontSize)
	, fontWeight(_T("fontWeight"), FW_NORMAL)
	, fontName(_T("fontName"), DEF_FONT_NAME)
{
	InitFont();
}

CSettings::~CSettings()
{
	DeleteFont();
}

void CSettings::SetUIFont(CHAR* lfFaceName, LONG lfWeight, LONG size)
{
	fontSize.Set(size);
	fontName.Set(lfFaceName);
	fontWeight.Set(lfWeight);
	InitFont();
}

void CSettings::InitFont()
{
	DeleteFont();

	ZeroMemory(&m_logFont, sizeof(LOGFONT));

	strncpy_s(m_logFont.lfFaceName, fontName.Get(), LF_FACESIZE);
	m_logFont.lfWeight = fontWeight.Get();
	HDC hdc = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
	m_logFont.lfHeight = -MulDiv(fontSize.Get(), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	m_logFont.lfQuality = CLEARTYPE_NATURAL_QUALITY; //ANTIALIASED_QUALITY

	m_Font = CreateFontIndirect(&m_logFont);

	DeleteDC(hdc);
}

void CSettings::DeleteFont()
{
	if (m_Font) {
		DeleteObject(m_Font);
		m_Font = NULL;
	}
}

void CSettings::RestoreWindPos(HWND hWnd)
{
	WINDOWPLACEMENT wpl;
	if (ReadBINARY(STR_APP_REG_VAL_WINDOWPOS, &wpl, sizeof(wpl))) 
	{
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
		WriteBINARY(STR_APP_REG_VAL_WINDOWPOS, &wpl, sizeof(wpl));
}
