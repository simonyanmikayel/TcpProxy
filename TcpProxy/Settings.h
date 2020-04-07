#pragma once

#include "RegKeyExt.h"
#include "ROUTE.h"

#define MAX_DATA_SIZE (1024*64)
#define MAX_ARRAY_SIZE (1024)

template<typename T>
struct ARR_VALUE
{
    friend class CSettings;
    const std::vector<T>& Get() { return val; }
	boolean Set(const std::vector<T>& val)
	{
		std::stringstream s;
		int count = int(val.size());
		if (count >= 0 && count < MAX_ARRAY_SIZE)
		{
			s << count << std::endl;
			for (int i = 0; i < val.size(); i++)
			{
				val[i].serialize(s);
			}
			const std::string& str = s.str();
			if (str.length() < MAX_DATA_SIZE - 1)
			{
				if (gSettings.WriteSTR(szRegKey, (char*)str.c_str()))
				{
					this->val = val;
					return true;
				}
			}
		}
		return false;
	}

private:
    ARR_VALUE(LPCTSTR sz);
    LPCTSTR szRegKey;
    std::vector<T> val;
};

struct INT_VALUE
{
    friend class CSettings;
    void Set(int val);
    int Get() { return val; }
private:
    INT_VALUE(LPCTSTR sz, int defVal);
    LPCTSTR szRegKey;
    int val;
};

struct STR_VALUE
{
	friend class CSettings;
	void Set(char* val);
	const char* Get() { return val.c_str(); }
private:
	STR_VALUE(LPCTSTR sz, const char* defVal);
	LPCTSTR szRegKey;
	std::string val;
};

class CSettings : public CRegKeyExt
{
public:
    CSettings();
    ~CSettings();

    INT_VALUE vertSplitterPos;
    ARR_VALUE<ROUTE> routes;
	INT_VALUE fontSize;
	STR_VALUE fontName;
	INT_VALUE fontWeight;


    void RestoreWindPos(HWND hWnd);
    void SaveWindPos(HWND hWnd);
	void SetUIFont(CHAR* lfFaceName, LONG lfWeight, LONG size);
	HFONT GetFont() { return m_Font; }

private:
	HFONT m_Font = 0;
	LOGFONT   m_logFont;
	void InitFont();
	void DeleteFont();
};

extern CSettings gSettings;