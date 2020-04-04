#include "stdafx.h"
#include "ConnectionManager.h"
#include "Heap.h"

#pragma warning(disable : 4702)

long CHeap::m_HeapCount = 0;
HEAP_ARRAY CHeap::m_HeapArray[MAX_HEAP_COUNT];

CHeap::CHeap(CConnectionManager *pMan, const char*  szHeapDescr, DWORD flOptions/*=0*/, SIZE_T dwInitialSize /*=0*/)
{
    InitializeCriticalSection(&m_cs);

  m_pMan = pMan;
  m_hHeap = ::HeapCreate(flOptions, dwInitialSize, 0);
  m_Index = InterlockedIncrement(&m_HeapCount)-1;
  if (m_Index < MAX_HEAP_COUNT)
  {
    m_HeapArray[m_Index].pHeap = this;
    strncpy_s(m_HeapArray[m_Index].szHeapDescr, szHeapDescr, MAX_HEAP_DESCR);
    m_HeapArray[m_Index].szHeapDescr[MAX_HEAP_DESCR] = 0;
  }
}

CHeap::~CHeap()
{
  if (m_Index < MAX_HEAP_COUNT)
    m_HeapArray[m_Index].pHeap = 0;

  if (m_hHeap)
    ::HeapDestroy(m_hHeap);

  DeleteCriticalSection(&m_cs);
}

void CHeap::Lock() {
    EnterCriticalSection(&m_cs);
}

void CHeap::Unlock() {
    LeaveCriticalSection(&m_cs);
}

void CHeap::InitElem(HEAP_ELEM * pElem, SIZE_T dwBytes, const char* szDescr)
{
    pElem->lpData = pElem;
    pElem->dwBytes = dwBytes;
    pElem->bAlloc = true;
    pElem->dwTick = GetTickCount();
    strncpy_s(pElem->szElemDescr, szDescr, MAX_HEAP_ELEM_DESCR);
    pElem->szElemDescr[MAX_HEAP_ELEM_DESCR] = 0;
}

LPVOID CHeap::Alloc(DWORD dwFlags,  // heap allocation control (as described in HeapAlloc)
                    SIZE_T dwBytes,  // number of bytes to allocate
                    const char* szDescr
                    )
{
  LPVOID pRet = 0;
  try 
  {
      pRet = ::HeapAlloc(m_hHeap, dwFlags | HEAP_GENERATE_EXCEPTIONS, dwBytes);
  }
  catch (...) 
  { 
    pRet = 0; 
  }

  if (!pRet && dwBytes)
  {
    CLogWriter::write("Exeption on HeapAlloc!!! dwBytes=%u Descr=%s\r\n", dwBytes, szDescr);
  }
  return pRet;
}

LPVOID CHeap::ReAlloc(DWORD dwFlags,  // heap reallocation options  (as described in HeapReAlloc)
                      LPVOID lpMem,   // pointer to memory to reallocate
                      SIZE_T dwBytes,  // number of bytes to reallocate
                      const char* szDescr
                      )
{
  LPVOID pRet = 0;
  try
  {
#ifndef _HEAP_LOG
    pRet = ::HeapReAlloc(m_hHeap, dwFlags | HEAP_GENERATE_EXCEPTIONS, lpMem, dwBytes);
#else
    HEAP_ELEM* pElem = (HEAP_ELEM*) ::HeapReAlloc(m_hHeap, dwFlags | HEAP_GENERATE_EXCEPTIONS, 
    (PBYTE)lpMem - sizeof(HEAP_ELEM), dwBytes + sizeof(HEAP_ELEM));
    if (pElem)
    {
      InitElem(pElem, dwBytes, szDescr);
      pRet = pElem + 1;    
    }
#endif
  }
  catch (...) { 
    pRet = 0; 
  }

  if (!pRet && dwBytes)
  {
    CLogWriter::write("Exeption on ReAlloc!!! lpMem=%u dwBytes=%u Descr=%s \r\n", lpMem, dwBytes, szDescr);
  }
  return pRet;
}

BOOL CHeap::Free(LPVOID lpMem) 
{
  if (lpMem == 0)
    return TRUE;

#ifdef _DEBUG
//CLogWriter::write("Heap Free lpMem=%u\r\n", lpMem);
#endif

  BOOL bRet = FALSE;
  try
  {
#ifndef _HEAP_LOG
    bRet = ::HeapFree(m_hHeap, 0, lpMem);
#else
    bRet = ::HeapFree(m_hHeap, 0, (PBYTE)lpMem - sizeof(HEAP_ELEM));
#endif
  }
  catch (...) { 
    bRet = 0;
  }

  if (!bRet)
  {
    CLogWriter::write("Exeption on Heap Free!!! lpMem=%u\r\n", lpMem);
  }
  return bRet;
}

BOOL CHeap::Validate(LPCVOID lpMem)
{
  BOOL bRet = FALSE;
  try
  {
    if (lpMem)
    {
#ifndef _HEAP_LOG
      bRet = ::HeapValidate(m_hHeap, 0, (PBYTE)lpMem);
#else
      bRet = ::HeapValidate(m_hHeap, 0, (PBYTE)lpMem - sizeof(HEAP_ELEM));
#endif
    }
    else
    {
      bRet = ::HeapValidate(m_hHeap, 0, 0);
    }
  }
  catch (...) {
    bRet = 0;
  }

  if (!bRet)
  {
    CLogWriter::write("Exeption on Heap Validate!!! lpMem=%u\r\n", lpMem);
  }
  return bRet;
}

#pragma warning(default : 4702)
