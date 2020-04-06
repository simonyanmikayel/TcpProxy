#include "stdafx.h"
#include "resource.h"
#include "Helpers.h"
#include "DlgRoute.h"

LRESULT DlgRoute::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_edtNmae.Attach(GetDlgItem(IDC_EDIT_NAME));
    m_edtLocalPort.Attach(GetDlgItem(IDC_EDIT_LOCAL_PORT));
    m_edtRemotePort.Attach(GetDlgItem(IDC_EDIT_REMOTE_PORT));
    m_edtRemoteAddr.Attach(GetDlgItem(IDC_EDIT_REMOTE_ADDR));

    m_edtNmae.SetWindowText(m_route.name.c_str());
    m_edtLocalPort.SetWindowText(Helpers::int2char(m_route.local_port));
    m_edtRemotePort.SetWindowText(Helpers::int2char(m_route.remote_port));
    m_edtRemoteAddr.SetWindowText(m_route.remote_addr.c_str());

    CenterWindow(GetParent());
    return TRUE;
}

LRESULT DlgRoute::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (wID == IDOK)
    {
        CString name; 
        m_edtNmae.GetWindowText(name); 
        m_route.name = name;

        CString local_port;
        m_edtLocalPort.GetWindowText(local_port);
        m_route.local_port = atoi(local_port);

        CString remote_port;
        m_edtRemotePort.GetWindowText(remote_port);
        m_route.remote_port = atoi(remote_port);

        CString remote_addr;
        m_edtRemoteAddr.GetWindowText(remote_addr); 
        m_route.remote_addr = remote_addr;
    }
    EndDialog(wID);
    return 0;
}
