#pragma once

class CRegKeyExt :public CRegKeyEx
{
public:
  CRegKeyExt(LPCTSTR lpszKeyName, HKEY hKeyParent = HKEY_CURRENT_USER)
  {
    Create(hKeyParent, lpszKeyName);
  }

  DWORD CheckData(LPCTSTR pszValueName, DWORD* lpcbData, DWORD* lpdwDataTye)
  {
      bool bRet = (ERROR_SUCCESS == QueryValue(pszValueName, lpdwDataTye, NULL, lpcbData));
      if (!bRet)
      {
          *lpcbData = 0;
          *lpdwDataTye = REG_NONE;
      }
      return bRet;
  }
      
  //DWORD
  bool WriteDWORD(LPCTSTR pszValueName, DWORD val)
  {
    return ERROR_SUCCESS == SetDWORDValue(pszValueName, val);
  }
  bool ReadDWORD(LPCTSTR pszValueName, DWORD& val, DWORD defVal = 0)
  {
    bool bRet = (ERROR_SUCCESS == QueryDWORDValue(pszValueName, val));
    if (!bRet)
      val = defVal;
    return bRet;
  }

  //ULONGLONG
  bool WriteQWORD(LPCTSTR pszValueName, ULONGLONG val)
  {
    return ERROR_SUCCESS == SetQWORDValue(pszValueName, val);
  }
  bool ReadQWORD(LPCTSTR pszValueName, ULONGLONG& val, ULONGLONG defVal = 0)
  {
    bool bRet = (ERROR_SUCCESS == QueryQWORDValue(pszValueName, val));
    if (!bRet)
      val = defVal;
    return bRet;
  }

  //LONG
  bool WriteLONG(LPCTSTR pszValueName, LONG val)
  {
    return ERROR_SUCCESS == WriteDWORD(pszValueName, (DWORD)val);
  }
  bool ReadLONG(LPCTSTR pszValueName, LONG& val, WORD defVal = 0)
  {
    DWORD dw = (LONG)val;
    bool bRet = ReadDWORD(pszValueName, dw, (DWORD)defVal);
    val = (LONG)dw;
    return bRet;
  }

  //int
  bool WriteINT(LPCTSTR pszValueName, int iValue)
  {
    return ERROR_SUCCESS == WriteDWORD(pszValueName, (DWORD)iValue);
  }
  bool ReadINT(LPCTSTR pszValueName, int& iValue, int defVal = 0)
  {
    DWORD dw = (DWORD)iValue;
    bool bRet = ReadDWORD(pszValueName, dw, (DWORD)defVal);
    iValue = (int)dw;
    return bRet;
  }

  //WORD
  bool WriteWORD(LPCTSTR pszValueName, WORD val)
  {
    return ERROR_SUCCESS == WriteDWORD(pszValueName, (DWORD)val);
  }
  bool ReadWORD(LPCTSTR pszValueName, WORD& val, WORD defVal = 0)
  {
    DWORD dw = (WORD)val;
    bool bRet = ReadDWORD(pszValueName, dw, (DWORD)defVal);
    val = (WORD)dw;
    return bRet;
  }

  //BYTE
  bool WriteBYTE(LPCTSTR pszValueName, BYTE val)
  {
    return ERROR_SUCCESS == WriteDWORD(pszValueName, (DWORD)val);
  }
  bool ReadBYTE(LPCTSTR pszValueName, BYTE& val, BYTE defVal = 0)
  {
    DWORD dw = (BYTE)val;
    bool bRet = ReadDWORD(pszValueName, dw, (DWORD)defVal);
    val = (BYTE)dw;
    return bRet;
  }

  //LPTSTR
  bool WriteSTR(LPCSTR pszValueName, LPCSTR val)
  {
    DWORD dwType = REG_SZ;
    return ERROR_SUCCESS == SetStringValue(pszValueName, val, dwType);
  }
  bool ReadSTR(LPCTSTR pszValueName, LPTSTR val, int cChars, LPTSTR defVal = 0)
  {
    int c = cChars; //
    ULONG* pnChars = (ULONG*)&cChars;
    bool bRet = ERROR_SUCCESS == QueryStringValue(pszValueName, val, pnChars);
    if (!bRet && defVal)
    {
#pragma warning(disable: 4996) // _tcsncpy depricated
      _tcsncpy(val, defVal, c);
      val[c - 1] = 0;
#pragma warning(default: 4996)
    }
    return bRet;
  }

  //void*
  bool ReadBINARY(LPCTSTR pszValueName, void* pValue, ULONG nBytes, bool init = false)
  {
    ULONG nBytesRead = nBytes; // initialize for sake of Win98
    if (init)
      ZeroMemory(pValue, nBytes);
    return ERROR_SUCCESS == QueryBinaryValue(pszValueName, pValue, &nBytesRead) && nBytesRead == nBytes;
  }
  bool WriteBINARY(LPCTSTR pszValueName, void* pValue, ULONG nBytes)
  {
    return ERROR_SUCCESS == SetBinaryValue(pszValueName, pValue, nBytes);
  }
};
