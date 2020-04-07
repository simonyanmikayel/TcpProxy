#include "stdafx.h"
#include "Helpers.h"
#include "Resource.h"
#include "comdef.h"
#include "MainFrame.h"

namespace Helpers
{
    void SysErrMessageBox(CHAR* lpFormat, ...)
    {
        DWORD err = GetLastError();
        CHAR* s = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            s, 0, NULL);

        va_list vl;
        va_start(vl, lpFormat);

        CHAR buf[1024];
        _vsntprintf_s(buf, _countof(buf), lpFormat, vl);

        ErrMessageBox(TEXT("%s\nErr: %d\n%s"), buf, err, s);
        va_end(vl);

        LocalFree(s);
    }

    void ErrMessageBox(CHAR* lpFormat, ...)
    {
        va_list vl;
        va_start(vl, lpFormat);
        CHAR* buf = new CHAR[1024];
        _vsntprintf_s(buf, 1023, 1023, lpFormat, vl);
        va_end(vl);

        if (IsWindow(hwndMain))
        {
            ::PostMessage(hwndMain, WM_SHOW_NGS, (WPARAM)buf, (LPARAM)0);
        }
        else
        {
            ::MessageBoxA(NULL, buf, "Error", MB_OK);
            delete[] buf;
        }
    }

    char* int2char(int i) 
    {
        static char buf[128];
        buf[0] = 0;
        _itoa_s(i, buf, _countof(buf), 10);
        return buf;
    }

    void UpdateStatusBar()
    {
        gMainFrame->UpdateStatusBar();
    }

    void AddMenu(HMENU hMenu, int& cMenu, int ID_MENU, LPCTCH str, bool disable)
    {
        DWORD dwFlags;
        dwFlags = MF_BYPOSITION | MF_STRING;
        if (disable)
            dwFlags |= MF_DISABLED;
        InsertMenu(hMenu, cMenu++, dwFlags, ID_MENU, str);
    }

};

