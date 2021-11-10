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
    m_closeWhenDataReceived.Attach(GetDlgItem(IDC_CHECK_CLOSE_ON_RECEIVE));    
    m_purgeReceivedPakage.Attach(GetDlgItem(IDC_CHECK_PURGE_RECV));
    m_sendHalfOfData.Attach(GetDlgItem(IDC_CHECK_SEND_HALF));
    m_closeRandomly.Attach(GetDlgItem(IDC_CHECK_CLOSE_RANDOMLY));

    m_edtNmae.SetWindowText(m_route.name.c_str());
    m_edtLocalPort.SetWindowText(Helpers::int2char(m_route.local_port));
    m_edtRemotePort.SetWindowText(Helpers::int2char(m_route.remote_port));
    m_edtRemoteAddr.SetWindowText(m_route.remote_addr.c_str());
    m_closeWhenDataReceived.SetCheck(m_route.closeWhenDataReceived ? BST_CHECKED : BST_UNCHECKED);
    m_purgeReceivedPakage.SetCheck(m_route.purgeReceivedPakage ? BST_CHECKED : BST_UNCHECKED);
    m_sendHalfOfData.SetCheck(m_route.sendHalfOfData ? BST_CHECKED : BST_UNCHECKED);
    m_closeRandomly.SetCheck(m_route.closeRandomly ? BST_CHECKED : BST_UNCHECKED);
    
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

        m_route.closeWhenDataReceived = m_closeWhenDataReceived.GetCheck();
        m_route.purgeReceivedPakage = m_purgeReceivedPakage.GetCheck();
        m_route.sendHalfOfData = m_sendHalfOfData.GetCheck();
        m_route.closeRandomly = m_closeRandomly.GetCheck();
        
    }
    EndDialog(wID);
    return 0;
}
