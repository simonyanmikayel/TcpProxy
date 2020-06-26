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
    ListView_SetExtendedListViewStyle(m_RouteList, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
    m_RouteList.InsertColumn(0, _T("Route name"), LVCFMT_LEFT, 300);
    m_RouteList.InsertColumn(1, _T("Local port"), LVCFMT_LEFT, 80);
    m_RouteList.InsertColumn(2, _T("Remote port"), LVCFMT_LEFT, 90);
    m_RouteList.InsertColumn(3, _T("Remote address"), LVCFMT_LEFT, 180);

    auto& r = gSettings.routes.Get();
    for (int i = 0; i < r.size(); i++)
    {
        routes.push_back(r[i]);
    }
    ShowRoutes(0);

    CenterWindow(GetParent());
    return TRUE;
}

bool myfunction(ROUTE &r1, ROUTE& r2)
{ 
    //return Helpers::CaseInsensitiveStringLesThan(r1.name, r2.name);
    return (r1.name < r2.name);
}

void DlgRouteTable::ShowRoutes(int nItem)
{
    if (nItem < 0)
        nItem = m_RouteList.GetSelectedIndex();
    if (nItem < 0)
        nItem = m_RouteList.GetItemCount() - 1;

    //sort routes
    for (int i = 0; i < routes.size(); i++)
    {
        routes[i].selected = (i == nItem);
    }
    std::sort(routes.begin(), routes.end(), myfunction);

    m_RouteList.DeleteAllItems();
    for (int i = 0; i < routes.size(); i++)
    {
        nItem = m_RouteList.GetItemCount();
        m_RouteList.AddItem(nItem, 0, "");
        m_RouteList.SetItem(nItem, 0, LVIF_TEXT, routes[i].name.c_str(), 0, 0, 0, 0);
        m_RouteList.SetItem(nItem, 1, LVIF_TEXT, Helpers::int2char(routes[i].local_port), 0, 0, 0, 0);
        m_RouteList.SetItem(nItem, 2, LVIF_TEXT, Helpers::int2char(routes[i].remote_port), 0, 0, 0, 0);
        m_RouteList.SetItem(nItem, 3, LVIF_TEXT, routes[i].remote_addr.c_str(), 0, 0, 0, 0);
        m_RouteList.SetCheckState(nItem, routes[i].enabled);
    }

    for (int i = 0; i < routes.size(); i++)
    {
        if (routes[i].selected)
        {
            nItem = i;
            m_RouteList.SelectItem(nItem);
            break;
        }
    }
}

LRESULT DlgRouteTable::OnCmdButtonAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    ROUTE r;
    DlgRoute dlg(&r);
    if (IDOK == dlg.DoModal())
    {
        routes.push_back(dlg.m_route);
        ShowRoutes((int)routes.size() - 1);
    }
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonEdit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nItem = m_RouteList.GetSelectedIndex();
    if (nItem >= 0)
    {
        ROUTE r;
        CString str;
        m_RouteList.GetItemText(nItem, 0, str);
        r.name = str;
        m_RouteList.GetItemText(nItem, 1, str);
        r.local_port = atoi(str);
        m_RouteList.GetItemText(nItem, 2, str);
        r.remote_port = atoi(str);
        m_RouteList.GetItemText(nItem, 3, str);
        r.remote_addr = str;

        DlgRoute dlg(&r);
        if (IDOK == dlg.DoModal())
        {
            routes[nItem] = dlg.m_route;
            ShowRoutes(nItem);
        }
    }
    return 0;
}

LRESULT DlgRouteTable::OnListDblClick(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    BOOL bHandled;
    OnCmdButtonEdit(0, 0, 0, bHandled);
    return 0;
}

LRESULT DlgRouteTable::OnListItemChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pnmh;
    BOOL oldStateChecked = ((pnmv->uOldState & LVIS_STATEIMAGEMASK) >> 12) - 1;
    BOOL newStateChecked = ((pnmv->uNewState & LVIS_STATEIMAGEMASK) >> 12) - 1;

    if (oldStateChecked >= 0 && oldStateChecked != newStateChecked)
        routes[pnmv->iItem].enabled = newStateChecked; // m_RouteList.GetCheckState(pnmv->iItem);
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonDelete(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nItem = m_RouteList.GetSelectedIndex();
    if (nItem >= 0)
    {
        routes.erase(routes.begin() + nItem);
        ShowRoutes(nItem > 0 ? nItem - 1 : 0);
    }
    return 0;
}

LRESULT DlgRouteTable::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (wID == IDOK)
    {
        //save settings
    }
    EndDialog(wID);
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonImport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CFileDialog dlg(TRUE);
    if (!dlg.DoModal() || !dlg.m_ofn.lpstrFile[0])
        return 0;

    FILE* fp;
    if (0 != fopen_s(&fp, dlg.m_ofn.lpstrFile, "r")) {
        Helpers::SysErrMessageBox(TEXT("Cannot open file %s"), dlg.m_ofn.lpstrFile);
        return 0;
    }

    routes.clear();
    unsigned int  enabled;
    unsigned int  local_port;
    unsigned int  remote_port;
    char str[512];
    
    while (3 == fscanf_s(fp, "%d;%d;%d;", &enabled, &local_port, &remote_port))
    {
        if (!fgets(str, sizeof(str), fp))
            break;
        str[511] = 0;
        size_t c = strlen(str);
        if (c<2)
            break;
        if (str[c - 1] == '\n')
        {
            str[c - 1] = 0;
        }
        ROUTE r;
        r.enabled = enabled;
        r.local_port = local_port;
        r.remote_port = remote_port;
        char* sep = strchr(str, ';');
        if (!sep)
            break;
        *sep = 0;
        r.name = str;
        r.remote_addr = sep+1;
        routes.push_back(r);
    }
    fclose(fp);

    ShowRoutes(0);
    return 0;
}

LRESULT DlgRouteTable::OnCmdButtonExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CFileDialog dlg(FALSE);
    if (!dlg.DoModal() || !dlg.m_ofn.lpstrFile[0])
        return 0;

    FILE* fp;
    if (0 != fopen_s(&fp, dlg.m_ofn.lpstrFile, "w")) {
        Helpers::SysErrMessageBox(TEXT("Cannot open file %s"), dlg.m_ofn.lpstrFile);
        return 0;
    }

    for (int i = 0; i < routes.size(); i++)
    {
        fprintf(fp, "%d;%d;%d;%s;%s\n",
            routes[i].enabled,
            routes[i].local_port,
            routes[i].remote_port,
            routes[i].name.c_str(),
            routes[i].remote_addr.c_str()
        );
    }
    fclose(fp);

    return 0;
}
