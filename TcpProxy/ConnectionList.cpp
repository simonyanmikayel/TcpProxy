#include "stdafx.h"
#include "ConnectionManager.h"
#include "Socket.h"
#include "ConnectionList.h"

CConnectionList::CConnectionList(CConnectionManager* pMan) : m_Heap(pMan, "CConnectionList - m_Heap") , m_ExtraHeap(pMan, "CConnectionList - m_ExtraHeap")
{
  m_pMan = pMan;
  m_pLastConn = 0;
  
  m_bTerm = FALSE;
}

CConnectionList::~CConnectionList()
{
}

void CConnectionList::CleanUp()
{
  m_bTerm = TRUE;
  CloseAll();
}

void CConnectionList::EnumConnections(ENUM_CONNECTIN_PROC EnumProc, PVOID pContext)
{
  m_Heap.Lock();
  DWORD dwNumber = 0;
  CConnection *pConn = m_pLastConn;
  while(pConn && EnumProc(pConn, dwNumber, FALSE, pContext))
  {
    pConn = pConn->m_pPrev;
    dwNumber ++;
  }
  m_Heap.Unlock();
}

CConnection* CConnectionList::CreateConnection(CSocket* pSocket)
{
  CConnection *pResult =
    (CConnection*)m_Heap.Alloc(HEAP_ZERO_MEMORY, sizeof(CConnection), "CreateConnection");
  if (pResult)
  {
    pResult->m_pMan      = m_pMan;
    pResult->m_pSocket   = pSocket;
  }
  return pResult;
}

CConnection* CConnectionList::AddConnection(CSocket* pSocket)
{
  CConnection *pResult = CreateConnection(pSocket);
  
  if(pResult)
    AddToList(pResult);
  
  return pResult;
}

void CConnectionList::AddToList(CConnection* pConn)
{
  m_Heap.Lock();
  
  pConn->m_pNext = 0;
  pConn->m_pPrev = m_pLastConn;
  
  if(m_pLastConn)
    m_pLastConn->m_pNext = pConn;
  
  m_pLastConn = pConn;
  
  pConn->m_btInList = 1;
  
  m_Heap.Unlock();
}

void CConnectionList::DeleteConnection(CConnection *pConn)
{
  FreeConnection(SkipConnection(pConn));
  
}

void CConnectionList::FreeConnection(CConnection *pConn)
{
  CleanConnection(pConn);
  m_Heap.Free(pConn);
}

void CConnectionList::CleanConnection(CConnection *pConn)
{
  if (pConn->m_pSocket)
  {
    m_pMan->m_SocketHolder.Release(pConn->m_pSocket, "CConnectionList::CleanConnection");
    pConn->m_pSocket = 0;
  }
}

void CConnectionList::CloseAll()
{
  m_Heap.Lock();
  CConnection *pConn = m_pLastConn;
  while (pConn)
  {
     m_pMan->m_SocketHolder.CloseSocket(pConn->m_pSocket, "CConnectionList::CheckTimeout");
    pConn = pConn->m_pPrev;
  }  
  m_Heap.Unlock();
}

PBYTE CConnectionList::SetStorageSize(CConnection &Conn, DWORD dwNewSize, const char* szDescr)
{
  PBYTE pResult = 0;
  if (dwNewSize == 0)
  {
    if (Conn.m_pbtDataStorage)
      m_ExtraHeap.Free(Conn.m_pbtDataStorage);
    Conn.m_pbtDataStorage      = 0;
    Conn.m_dwStorageActualSize = 0;
  }
  else       
  {
    DWORD dwNewActualSize;
    if (dwNewSize < 0x2FFFFF)
      dwNewActualSize = ((dwNewSize >> 9) << 9) + 512;
    else
      dwNewActualSize = dwNewSize;

    if(Conn.m_pbtDataStorage)
    {
      if (dwNewActualSize == Conn.m_dwStorageActualSize)
        pResult = 
        Conn.m_pbtDataStorage;
      else
        pResult = 
        (PBYTE)m_ExtraHeap.ReAlloc(0,  Conn.m_pbtDataStorage, dwNewActualSize, szDescr);
      
    }
    else
    {
      pResult =
        (PBYTE)m_ExtraHeap.Alloc(0, dwNewActualSize, szDescr);
    }
    
    if(pResult)
    {
      Conn.m_pbtDataStorage = pResult;
      Conn.m_dwStorageActualSize = dwNewActualSize;
    }
  }
  
  return pResult;
}

PBYTE CConnectionList::SetConnExtraSize(CConnection &Conn, DWORD dwNewSize, const char* szDescr)
{
  PBYTE pResult = 0;
  if (dwNewSize == 0)
  {
    if (Conn.m_pbtExtraData)
      m_ExtraHeap.Free(Conn.m_pbtExtraData);
    Conn.m_pbtExtraData     = 0;
    Conn.m_dwExtraDataSize  = 0;
    Conn.m_dwExtraDataActualSize = 0;
  }
  else       
  {
    DWORD dwNewActualSize;
    if (dwNewSize < 0x2FFFFF)
      dwNewActualSize = ((dwNewSize >> 9) << 9) + 512;
    else
      dwNewActualSize = dwNewSize;

    if(Conn.m_pbtExtraData)
    {
      if (dwNewActualSize == Conn.m_dwExtraDataActualSize)
        pResult = 
        Conn.m_pbtExtraData;
      else
        pResult = 
        (PBYTE)m_ExtraHeap.ReAlloc(0,  Conn.m_pbtExtraData, dwNewActualSize, szDescr);
      
    }
    else
    {
      pResult =
        (PBYTE)m_ExtraHeap.Alloc(0, dwNewActualSize, szDescr);
    }
    
    if(pResult)
    {
      Conn.m_pbtExtraData = pResult;
      Conn.m_dwExtraDataSize = dwNewSize;
      Conn.m_dwExtraDataActualSize = dwNewActualSize;
    }
  }
  
  return pResult;
}

PBYTE CConnectionList::AddExtraTail(CConnection &Conn, DWORD dwTailSize, const char* szDescr)
{
  PBYTE pResult = 0;
  DWORD dwOldSize = Conn.m_dwExtraDataSize;
  if(SetConnExtraSize(Conn, dwOldSize + dwTailSize, szDescr))
  {
    pResult = Conn.m_pbtExtraData + dwOldSize;
  }
  return pResult;
}

PBYTE CConnectionList::AddExtraData(CConnection &Conn, void* pBuf, DWORD dwBufLen, const char* szDescr)
{
  PBYTE pResult = AddExtraTail(Conn, dwBufLen, szDescr);
  if (pResult)
  {
    memcpy(pResult, pBuf, dwBufLen);
  }
  return pResult;
}

CConnection* CConnectionList::SkipConnection(CConnection *pConn)
{
  if (pConn->m_btInList)
  {    
    pConn->m_btInList = 0;

    m_Heap.Lock();
    
    if(pConn == m_pLastConn)//this is the last connection
      m_pLastConn = pConn->m_pPrev;
    
    if(pConn->m_pPrev)
      pConn->m_pPrev->m_pNext = pConn->m_pNext;
    if(pConn->m_pNext)
      pConn->m_pNext->m_pPrev = pConn->m_pPrev;

    pConn->m_pNext = 0;
    pConn->m_pPrev = 0;
    
    m_Heap.Unlock();
  }
  return pConn;
}



