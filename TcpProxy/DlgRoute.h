#pragma once

#include "ROUTE.h"

class DlgRoute : public CDialogImpl<DlgRoute>
{
public:
    DlgRoute(const ROUTE* r) {
        if (r) m_route = *r;
    }
    enum { IDD = IDD_ROUTE };

    BEGIN_MSG_MAP(DlgSettings)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    CEdit   m_edtNmae;
    CEdit   m_edtLocalPort;
    CEdit   m_edtRemotePort;
    CEdit   m_edtRemoteAddr;

    ROUTE m_route;

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

