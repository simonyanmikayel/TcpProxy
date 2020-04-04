#pragma once

#include "RegKeyExt.h"

class CSettings : public CRegKeyExt
{
public:
    CSettings();
    ~CSettings();

    void RestoreWindPos(HWND hWnd);
    void SaveWindPos(HWND hWnd);

private:
};

extern CSettings gSettings;