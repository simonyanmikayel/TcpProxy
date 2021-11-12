#include "stdafx.h"
#include "DlgProgress.h"
#include "Helpers.h"
#include "Archive.h"
#include "Settings.h"
#include "FileMap.h"

#pragma warning( push )
#pragma warning(disable:4477) //string '%d' requires an argument of type 'int *', but variadic argument 2 has type 'char *'

DlgProgress::DlgProgress(TASK_TYPE taskType, LPSTR lpstrFile)
{
    m_taskType = taskType;
    m_pTaskThread = new TaskThread(taskType, lpstrFile);
}


DlgProgress::~DlgProgress()
{
    delete m_pTaskThread;
}

LRESULT DlgProgress::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (!m_pTaskThread->IsOK())
    {
        End(IDCANCEL);
        return FALSE;
    }

    m_pTaskThread->StartWork(NULL);
    SetTimer(1, 2000);

    m_ctrlInfo.Attach(GetDlgItem(IDC_STATIC_INFO));
    m_ctrlProgress.Attach(GetDlgItem(IDC_PROGRESS));
    m_ctrlProgress.SetRange(0, 100);

    CenterWindow(GetParent());
    return TRUE;
}

LRESULT DlgProgress::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    if (wParam == 1)
    {
        if (m_pTaskThread->IsWorking())
        {
            double total, cur;
            m_pTaskThread->GetProgress(total, cur);
            CString strMsg;
            strMsg.Format(_T("%llu of %llu"), (long long)cur, (long long)total);
            m_ctrlInfo.SetWindowText(strMsg);
            if (total)
            {
                double progress = 100.0 * (cur / total);
                m_ctrlProgress.SetPos((int)progress);
                Helpers::UpdateStatusBar();
            }

        }
        else
        {
            End(IDOK);
        }

    }
    return 0;
}

LRESULT DlgProgress::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    End(IDCANCEL);
    return 0;
}

void DlgProgress::End(int wID)
{
    KillTimer(1);
    m_pTaskThread->StopWork();
    if (m_taskType == TASK_TYPE::IMPORT)
    {
        ::PostMessage(hwndMain, WM_INPORT_TASK, 1, 0);
    }
    EndDialog(wID);

}

///////////////////////////////////////////////////
TaskThread::TaskThread(TASK_TYPE taskType, LPSTR lpstrFile)
{
    m_taskType = taskType;
    m_isOK = false;
    m_fp = NULL;
    m_progress = 0;
    m_totoal = 1;

    if (lpstrFile == 0)
    {
        INT_PTR nRet;
        if (m_taskType == TASK_TYPE::IMPORT)
        {
            CFileDialog dlg(TRUE);
            nRet = dlg.DoModal();
            m_strFilePath = dlg.m_ofn.lpstrFile;
        }
        else
        {
            //LPCTSTR lpcstrTextFilter =
            //  _T("Text Files (*.txt)\0*.txt\0")
            //  _T("All Files (*.*)\0*.*\0")
            //  _T("");
            CFileDialog dlg(FALSE);// , NULL, _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, lpcstrTextFilter);
            nRet = dlg.DoModal();
            m_strFilePath = dlg.m_ofn.lpstrFile;
        }
    }
    else
    {
        m_strFilePath = lpstrFile;
    }

    int chPos = m_strFilePath.ReverseFind('\\');
    if (chPos < 0)
        chPos = m_strFilePath.ReverseFind('/');
    if (chPos > 0)
        m_strFileName = m_strFilePath.Mid(chPos + 1);
    else
        m_strFileName = m_strFilePath;
    chPos = m_strFileName.ReverseFind('.');
    if (chPos > 0) {
        m_strFileExt = m_strFileName.Mid(chPos + 1);
        m_strFileNameWithoutExt = m_strFileName.Mid(0, chPos);
    }
    else
        m_strFileNameWithoutExt = m_strFileName;

    m_fp = NULL;
    if (!m_strFilePath.IsEmpty())
    {
        if (m_taskType == TASK_TYPE::IMPORT)
        {
            if (0 == fopen_s(&m_fp, m_strFilePath, "rb")) {
                m_isOK = true;
            }
            else
            {
                Helpers::SysErrMessageBox(TEXT("Cannot open file %s"), m_strFilePath);
                m_fp = NULL;
            }
        }
        else
        {
            if (0 == fopen_s(&m_fp, m_strFilePath, "wb")) {
                m_isOK = true;
            }
            else
            {
                Helpers::SysErrMessageBox(TEXT("Cannot create file %s"), m_strFilePath);
                m_fp = NULL;
            }
        }
    }
}

TaskThread::~TaskThread()
{
    if (m_fp)
        fclose(m_fp);
}

void TaskThread::Terminate()
{
}

void TaskThread::GetProgress(double& total, double& cur)
{
    total = m_totoal;
    cur = m_progress;
}

void TaskThread::Work(LPVOID pWorkParam)
{
    if (m_isOK)
    {
        if (m_taskType == TASK_TYPE::IMPORT)
            FileImportLog();
        else
            FileExportLog();
    }
}

bool TaskThread::exportLog(LOG_NODE* pNode)
{
    m_progress++;
    bool ss = true;
    DWORD childCount = pNode->childCount;
    ss = ss && (1 == fwrite(&childCount, sizeof(DWORD), 1, m_fp));
    DWORD size = pNode->nodeSize() - sizeof(LOG_NODE);
    ss = ss && (1 == fwrite(&size, sizeof(DWORD), 1, m_fp));
    DWORD data_type = (DWORD)pNode->data_type;
    ss = ss && (1 == fwrite(&data_type, sizeof(DWORD), 1, m_fp));
    if (gArchive.getRootNode() != pNode)
        ss = ss && (1 == fwrite(((char*)pNode) + sizeof(LOG_NODE), size, 1, m_fp));

    LOG_NODE* pChildNode = pNode->firstChild;
    while (pChildNode && ss && childCount && IsWorking())
    {
        ss = ss && exportLog(pChildNode);
        pChildNode = pChildNode->nextSibling;
        childCount--;
    }
    return ss;
}

void TaskThread::FileExportLog()
{
    m_totoal = gArchive.getNodeCount();
    bool ss = exportLog(gArchive.getRootNode());
    if (!ss)
    {
        Helpers::ErrMessageBox(TEXT("Failed to export log record %d"), m_progress);
    }
}

bool TaskThread::importNode(LOG_NODE* pNode)
{
    bool ss = true;
    DWORD childCount;
    DWORD size = 0;
    DWORD data_type = 0;    
    LOG_NODE* pChildNode = nullptr;
    ss = ss && (1 == fread(&childCount, sizeof(DWORD), 1, m_fp));
    ss = ss && (1 == fread(&size, sizeof(DWORD), 1, m_fp));
    ss = ss && (1 == fread(&data_type, sizeof(DWORD), 1, m_fp));
    if (pNode)
        pChildNode = gArchive.importNode(size, m_fp, pNode, data_type);
    else
        pChildNode = gArchive.getRootNode();

    ss = ss && (pChildNode != nullptr);

    m_progress = ftell(m_fp);
    while (childCount && ss && IsWorking())
    {
        ss = ss && importNode(pChildNode);
        childCount--;
    }
    return ss;
}

void TaskThread::FileImportLog()
{
    fseek(m_fp, 0, SEEK_END);
    m_totoal = ftell(m_fp);
    fseek(m_fp, 0, SEEK_SET);
    m_progress = ftell(m_fp);

    ::SendMessage(hwndMain, WM_INPORT_TASK, 0, 0); //this will stop logging and will clear log

    bool ss = importNode(nullptr);
    if (!ss)
    {
        Helpers::ErrMessageBox(TEXT("Failed to import log record %d"), gArchive.getNodeCount());
    }
}


#pragma warning(pop)
