#pragma once

#define MAX_HEAP_COUNT 32
#define MAX_HEAP_DESCR 63
#define MAX_HEAP_ELEM_DESCR 63

struct HEAP_ELEM
{
  PVOID lpData;   // Pointer retuned by HeapAlloc or HeapReAlloc
  SIZE_T dwBytes; // size passed to HeapAlloc or HeapReAlloc
  bool  bAlloc;
  DWORD dwTick;
  char szElemDescr[MAX_HEAP_ELEM_DESCR + 1];
};

class CHeap;
class CConnectionManager;
struct HEAP_ARRAY
{
  CHeap* pHeap;
  char szHeapDescr[MAX_HEAP_DESCR + 1];
};

class CHeap
{
  friend class HEAP_DATA;
public:
	CHeap(CConnectionManager *pMan, const char* szHeapDescr, DWORD flOptions =0, SIZE_T dwInitialSize = 0);
	~CHeap();

  LPVOID  Alloc(DWORD dwFlags,  SIZE_T dwBytes,  const char* szDescr);
  LPVOID  ReAlloc(DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes, const char* szDescr);
  BOOL    Free(LPVOID lpMem);
  BOOL    Validate(LPCVOID lpMem);

  void Lock();
  void Unlock();

private:
  void InitElem(HEAP_ELEM * pElem, SIZE_T dwBytes, const char* szDescr);
  int    m_Index; // index in m_HeapArray
  CConnectionManager *m_pMan; // ensure that all heap created after CConnectionManager
  static long m_HeapCount;
  static HEAP_ARRAY m_HeapArray[MAX_HEAP_COUNT];

  CRITICAL_SECTION   m_cs;
  HANDLE  m_hHeap;
};

