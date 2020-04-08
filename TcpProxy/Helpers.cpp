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

    CHAR* str_format_int_grouped(__int64 num)
    {
        static CHAR dst[16];
        CHAR src[16];
        char* p_src = src;
        char* p_dst = dst;

        const char separator = ',';
        int num_len, commas;

        num_len = sprintf_s(src, _countof(src), "%lld", num);

        if (*p_src == '-') {
            *p_dst++ = *p_src++;
            num_len--;
        }

        for (commas = 2 - num_len % 3; *p_src; commas = (commas + 1) % 3) {
            *p_dst++ = *p_src++;
            if (commas == 1) {
                *p_dst++ = separator;
            }
        }
        *--p_dst = '\0';
        return dst;
    }
    
    size_t HexDump(BYTE* pBuf, size_t cBuf, const BYTE* pData, size_t cData, int cColumn)
    {
        const char* hex = "0123456789ABCDEF";
        size_t i, j, k = 0;

        for (i = 0; i < cData; ) {
            size_t i0 = i;
            for (j = 0; j < cColumn && i < cData && k < cBuf; j++, i++)
            {
                if (k < cBuf)
                    pBuf[k++] = hex[(pData[i] >> 4) & 0x0F];
                if (k < cBuf)
                    pBuf[k++] = hex[(pData[i]) & 0x0F];
                if (k < cBuf)
                    pBuf[k++] = ' ';
            }
            if (k < cBuf)
                pBuf[k++] = '\t';
            i = i0;
            for (j = 0; j < cColumn && i < cData && k < cBuf; j++, i++)
            {
                if (k < cBuf)
                    pBuf[k++] = pData[i] >= ' ' ? pData[i] : '.';
            }
            if (k < cBuf)
                pBuf[k++] = '\r';
            if (k < cBuf)
                pBuf[k++] = '\n';
        }
        return k;
    }
};

