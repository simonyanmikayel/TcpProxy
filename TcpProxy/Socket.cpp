#include "stdafx.h"
#include "ConnectionManager.h"
#include "Socket.h"

DWORD CSocketHolder::m_dwCount = 0;

CSocketHolder::CSocketHolder(CConnectionManager* pMan): m_Heap(pMan, "CSocketHolder") 
{
  m_pMan = pMan;
}

CSocketHolder::~CSocketHolder()
{
}

CSocket* CSocketHolder::CreateSocket(SOCKET s)
{
  CSocket* pSocket = (CSocket*)m_Heap.Alloc(HEAP_ZERO_MEMORY, sizeof(CSocket), "CSocketHolder::CreateSocket");
  if (pSocket)
  {
    pSocket->m_s = s;
    pSocket->m_dwRef = 1;
    m_dwCount ++;
  }
  return pSocket;
}

DWORD  CSocketHolder::AddRef(CSocket* pSocket)
{
  if (!pSocket)
    return 0;

  return InterlockedIncrement(&pSocket->m_dwRef);
}

DWORD  CSocketHolder::Release(CSocket* pSocket, LPCSTR szDescr /*= 0*/)
{
  if (!pSocket)
    return 0;

  DWORD dwRet = InterlockedDecrement(&pSocket->m_dwRef);
  if (!dwRet)
  { 
    if (pSocket->m_s != INVALID_SOCKET)
    {
      //printf("CSocketHolder::Release %u : %s \r\n", pSocket->m_s, szDescr ? szDescr : "");
      closesocket(pSocket->m_s);
    }
    m_Heap.Free(pSocket);
    m_dwCount --;
  }

  return dwRet;
}

BOOL  CSocketHolder::CloseSocket(CSocket* pSocket, LPCSTR szDescr /*= 0*/)
{
  BOOL bRet = FALSE;
  if (pSocket && pSocket->m_s != INVALID_SOCKET)
  {
    m_Heap.Lock();
    if (pSocket->m_s != INVALID_SOCKET) // chek again to prevent double closing within concurent threads
    {
      //printf("CSocketHolder::CloseSocket %u : %s \r\n", pSocket->m_s, szDescr ? szDescr : "");
      closesocket(pSocket->m_s);
      pSocket->m_s = INVALID_SOCKET;
      bRet = TRUE;
    }
    m_Heap.Unlock();
  }
  return bRet;
}

DWORD  CSocketHolder::SR(IO_TYPE io_type, CSocket* pSocket, PVOID pData, DWORD dwSize, CONNECTION* pConn)
{
  DWORD dwFlags = 0;
  DWORD dwBytes = 0;
  DWORD dwError = IOM_SOCKET_ERROR;
  pConn->m_dwTransfer = dwSize;
  pConn->m_io_type = io_type;

  if (pSocket && pSocket->m_s != INVALID_SOCKET)
  {
    m_Heap.Lock();
    if (pSocket->m_s != INVALID_SOCKET) // chek btClosed again to prevent double closing within concurent threads
    {
      WSABUF buf = {dwSize, (char*)pData};
      int  Result = 0;

      if (pConn->m_io_type == SEND)
      {
        Result = WSASend(pSocket->m_s, &buf, 1, &dwBytes, 0,  pConn, 0);
      }
      else if (pConn->m_io_type == RECV)
      {
        Result = WSARecv(pSocket->m_s, &buf, 1, &dwBytes, &dwFlags, pConn, 0);
      }
      else
      {
        //TODO Result = ConnectEx(pSocket->m_s, &buf, 1, &dwBytes, &dwFlags, pConn, 0);
      }

      if (Result == SOCKET_ERROR)
        dwError = GetLastError();
      else
        dwError = 0;
    }
    m_Heap.Unlock();
  }

  if (dwError && dwError != ERROR_IO_PENDING)
    PostQueuedCompletionStatus(m_pMan->GetPort(), dwBytes, dwError, pConn);

  return dwError;
}

BOOL CSocketHolder::GetPeerName(CSocket* pSocket, SOCKADDR_IN& SockAddr)
{
  int namelen = sizeof(SOCKADDR_IN);
  if ( pSocket && (SOCKET_ERROR  != getpeername(pSocket->m_s, (sockaddr*)&SockAddr, &namelen)))
    return TRUE;
  else
    return FALSE;
}

BOOL CSocketHolder::GetSockName(CSocket* pSocket, SOCKADDR_IN& SockAddr)
{
  int namelen = sizeof(SOCKADDR_IN);
  if ( pSocket && (SOCKET_ERROR  != getsockname(pSocket->m_s, (sockaddr*)&SockAddr, &namelen)))
    return TRUE;
  else
    return FALSE;
}
