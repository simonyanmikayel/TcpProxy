#pragma once

#include "ROUTE.h"

class DlgRoute : public CDialogImpl<DlgRoute>
{
public:
    DlgRoute(const ROUTE* r) {
        if (r) m_route = *r;
    }
    enum { IDD = IDD_ROUTE };

    BEGIN_MSG_MAP(DlgRouteTable)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    CEdit   m_edtNmae;
    CEdit   m_edtLocalPort;
    CEdit   m_edtRemotePort;
    CEdit   m_edtRemoteAddr;
    CButton m_closeWhenDataReceived;
    CButton m_purgeReceivedPakage;
    CButton m_sendHalfOfData;
    CButton m_closeRandomly;

    ROUTE m_route;

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

