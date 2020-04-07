#pragma once
#include "Settings.h"
#include "FontPreviewCombo.h"

class DlgSettings : public CDialogImpl<DlgSettings>
{
public:
  enum { IDD = IDD_SETTINGS };

  BEGIN_MSG_MAP(DlgSettings)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
    COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    COMMAND_HANDLER(IDC_BTN_FONT, BN_CLICKED, OnBnClickedBtnFont)
    REFLECT_NOTIFICATIONS()
  END_MSG_MAP()

  // Handler prototypes (uncomment arguments if needed):
  //	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
  //	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
  //	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnBnClickedBtnFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  void SetFontLabel();
  CStatic m_lblFont;
  CButton m_btnFont;

  DWORD     m_FontSize;
  CHAR      m_FaceName[LF_FACESIZE];
  DWORD     m_lfWeight;
};
