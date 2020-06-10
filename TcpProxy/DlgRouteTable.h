#pragma once

#include "ROUTE.h"


class DlgRouteTable : public CDialogImpl<DlgRouteTable>
{
public:
    enum { IDD = IDD_ROUTE_TABLE };

    BEGIN_MSG_MAP(DlgRouteTable)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_HANDLER(IDC_BUTTON_ADD, BN_CLICKED, OnCmdButtonAdd)
        COMMAND_HANDLER(IDC_BUTTON_EDIT, BN_CLICKED, OnCmdButtonEdit)
        COMMAND_HANDLER(IDC_BUTTON_DELETE, BN_CLICKED, OnCmdButtonDelete)
        COMMAND_HANDLER(IDC_BUTTON_IMPORT, BN_CLICKED, OnCmdButtonImport)
        COMMAND_HANDLER(IDC_BUTTON_EXPORT, BN_CLICKED, OnCmdButtonExport)
        NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblClick)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    CListViewCtrl m_RouteList;
    std::vector<ROUTE> routes;

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCmdButtonAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCmdButtonEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCmdButtonDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCmdButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnCmdButtonExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnDblClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/);

    //void GetRoute(ROUTE& r, int iItem);
    //void SetRoute(const ROUTE& r, int iItem);
    //void AddRoute(const ROUTE& r);
    //void EdotRoute();
    void ShowRoutes(int nItem);
};

