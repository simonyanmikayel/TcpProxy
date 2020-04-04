#pragma once

#include "Heap.h"

class CConnectionManager;
class CSocket;

enum IO_TYPE {CONNECT, RECV, SEND};

struct CONNECTION : public OVERLAPPED
{
    DWORD               m_dwTransfer;
    IO_TYPE             m_io_type;
    CConnectionManager* m_pMan;
};

class CConnection : public CONNECTION
{
public:
    CSocket*     m_pSocket;
    BYTE         m_Err;

    BYTE         m_btInList : 1;

    CConnection* m_pPrev;
    CConnection* m_pNext;

    DWORD       m_dwExtraDataSize;
    PBYTE       m_pbtExtraData;
    DWORD       m_dwExtraDataActualSize;

    PBYTE       m_pbtDataStorage;
    DWORD       m_dwStorageActualSize;
    PBYTE       m_pbtDataBuf; // points to m_pbtDataStorage or m_btData depend on data size
    DWORD       m_dwDataSize;

};

class CSocket
{
  friend class  CSocketHolder;
  friend class  CLogWriter;

  SOCKET m_s;
  long  m_dwRef;//volatile 
public:
  BOOL IsEqual(CSocket *p){if (p) return p->m_s == m_s; else return FALSE;}
};

class CSocketHolder
{
public:
  CSocketHolder(CConnectionManager* pMan);
  ~CSocketHolder();

  CSocket*  CreateSocket(SOCKET s);
  BOOL      CloseSocket (CSocket* pSocket, LPCSTR szDescr = 0);
  DWORD     AddRef      (CSocket* pSocket);
  DWORD     Release     (CSocket* pSocket, LPCSTR szDescr = 0);

  DWORD     SR(IO_TYPE io_type, CSocket* pSocket, PVOID pData, DWORD dwSize, CONNECTION* pConn);

  static BOOL      GetPeerName(CSocket* pSocket, SOCKADDR_IN& SockAddr);
  BOOL      GetSockName(CSocket* pSocket, SOCKADDR_IN& SockAddr);
  DWORD     GetSocketCount(){return m_dwCount;}

#ifdef _DBG_SOCKET
  DWORD     GetSocketCount1();
  void ResetConn(CSocket* pSocket);
  CSocket *m_pLast;
#endif
private:
  static DWORD m_dwCount;
  CConnectionManager* m_pMan;
  CHeap m_Heap;
};


