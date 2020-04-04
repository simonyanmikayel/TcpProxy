#pragma once

#include "Heap.h"

typedef BOOL (*ENUM_CONNECTIN_PROC)(CConnection *pConn, DWORD dwNumber, BOOL bOut, PVOID pContext);

class CConnectionList
{
    friend class CConnection;
public:
    CConnectionList(CConnectionManager* pMan);
    ~CConnectionList();

    CConnection* CreateConnection(CSocket* pSocket);
    CConnection* AddConnection(CSocket* pSocket);
    void         AddToList(CConnection* pConn);
    CConnection*  SkipConnection(CConnection *pConn);
    void        DeleteConnection(CConnection *pConn);
    void        CleanConnection(CConnection *pConn);
    void        FreeConnection(CConnection *pConn);
    void        CloseAll();
    PBYTE       SetConnExtraSize(CConnection &Conn, DWORD dwNewSize, const char* szDescr);
    PBYTE       SetStorageSize(CConnection &Conn, DWORD dwNewSize, const char* szDescr);
    PBYTE       AddExtraTail(CConnection &Conn, DWORD dwTailSize, const char* szDescr);
    PBYTE       AddExtraData(CConnection &Conn, void* pBuf, DWORD dwBufLen, const char* szDescr);
    void        CleanUp();
    void        EnumConnections(ENUM_CONNECTIN_PROC EnumProc, PVOID pContext);
    BOOL        ValidateConnHeap (LPCVOID lpMem){return m_Heap.Validate(lpMem);}
    BOOL        ValidateExtraHeap(LPCVOID lpMem){return m_ExtraHeap.Validate(lpMem);}
private:
    CHeap               m_Heap;
    CHeap               m_ExtraHeap;
    BOOL                m_bTerm;//if TRUE the thread exits
    CConnectionManager  *m_pMan;
    CConnection         *m_pLastConn;
};
