#include "stdafx.h"
#include "resource.h"
#include "DlgRoute.h"
#include "DlgRouteTable.h"
#include "Settings.h"
#include "Helpers.h"

LRESULT DlgRouteTable::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_RouteList.Attach(GetDlgItem(IDC_ROUTE_LIST));
    //m_RouteList.ModifyStyleEx(0, LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyle(m_RouteList, LVS_EX_FULLROWSELECT);
    m_RouteList.InsertColumn(0, _T("Route name"), LVCFMT_LEFT, 200);
    m_RouteList.InsertColumn(1, _T("Local port"), LVCFMT_LEFT, 80);
    m_RouteList.InsertColumn(2, _T("Remote port"), LVCFMT_LEFT, 90);
    m_RouteList.InsertColumn(3, _T("Remote address"), LVCFMT_LEFT, 210);

    auto& r = gSettings.routes.Get();
    for (int i = 0; i < r.size(); i++)
    {
        AddRoute(r[i]);
    }
    m_RouteList.SelectItem(0);

    CenterWindow(GetParent());
    return TRUE;
}

LRESULT DlgRouteTable::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (wID == IDOK)
    {
        //save settings
    }
    routes.clear();
    int count = m_RouteList.GetItemCount();
    if (count > 0)
    {
        routes.reserve(count);
        ROUTE r;
        for (int i = 0; i < count; i++)
        {
            GetRoute(r, i);
            routes.push_back(r);
        }
    }
    EndDialog(wID);
    return 0;
}

void DlgRouteTable::GetRoute(ROUTE& r, int i)
{
    CString str;
    m_RouteList.GetItemText(i, 0, str);
    r.name = str;
    m_RouteList.GetItemText(i, 1, str);
    r.local_port = atoi(str);
    m_RouteList.GetItemText(i, 2, str);
    r.remote_port = atoi(str);
    m_RouteList.GetItemText(i, 3, str);
    r.remote_addr = str;
}

void DlgRouteTable::SetRoute(const ROUTE& r, int nItem)
{
    m_RouteList.SetItem(nItem, 0, LVIF_TEXT, r.name.c_str(), 0, 0, 0, 0);
    m_RouteList.SetItem(nItem, 1, LVIF_TEXT, Helpers::int2char(r.local_port), 0, 0, 0, 0);
    m_RouteList.SetItem(nItem, 2, LVIF_TEXT, Helpers::int2char(r.remote_port), 0, 0, 0, 0);
    m_RouteList.SetItem(nItem, 3, LVIF_TEXT, r.remote_addr.c_str(), 0, 0, 0, 0);
    m_RouteList.SelectItem(nItem);
}

void DlgRouteTable::AddRoute(const ROUTE& r)
{
    int nItem = m_RouteList.GetItemCount();
    m_RouteList.AddItem(nItem, 0, "");
    SetRoute(r, nItem);
}

LRESULT DlgRouteTable::OnCmdButtonAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ROUTE r;
    DlgRoute dlg(&r);
    if (IDOK == dlg.DoModal())
    {
        AddRoute(dlg.m_route);
    }
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nItem = m_RouteList.GetSelectedIndex();
    if (nItem >= 0)
    {
        ROUTE r;
        GetRoute(r, nItem);
        DlgRoute dlg(&r);
        if (IDOK == dlg.DoModal())
        {
            SetRoute(dlg.m_route, nItem);
        }
    }
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nItem = m_RouteList.GetSelectedIndex();
    if (nItem >= 0)
    {
        m_RouteList.DeleteItem(nItem);
        if (nItem > 0)
            m_RouteList.SelectItem(nItem - 1);
        else if (m_RouteList.GetItemCount() > 0)
            m_RouteList.SelectItem(0);
    }
    return 0;
}
