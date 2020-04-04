#include "stdafx.h"
#include "ConnectionManager.h"

#define DEF_CM_INITTHREADCOUNT	1    //initially 1 parallel threads
#define DEF_CM_MINTHREADCOUNT	1    //min. threads if absolute inactivity
#define DEF_CM_MAXTHREADCOUNT	32  //max. threads
#define DEF_CM_MAXTHREADIDLE	3600000   //1 hour in ms -s

CConnectionManager::CConnectionManager() :
      m_dwAllThreadCount(0)
    , m_dwConnThreadCount(0)
    , m_dwActiveThreadCount(0)
    , m_SocketHolder(this)
    , m_ConnList(this)
{
	InitializeCriticalSection(&m_cs);

    m_hThreadsStarted = CreateEvent(0, TRUE, FALSE, 0);
    m_hTerminate = CreateEvent(0, TRUE, FALSE, 0);
    m_hAllThreadsClosed = CreateEvent(0, TRUE, FALSE, 0);

    m_hPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);

    for (WORD w = 0; w < DEF_CM_MINTHREADCOUNT; w++) {
        StartThread(IoCompProc, this);
    }

}

CConnectionManager::~CConnectionManager()
{
    //TODO
}

void CConnectionManager::AddRoute(ROUTE* pRoute)
{
    ARG_ACCEPT_THREAD* pArg = new ARG_ACCEPT_THREAD(this, pRoute);
    StartThread(AcceptProc, pArg);
}

DWORD CALLBACK CConnectionManager::AcceptProc(LPVOID lpThreadParameter)
{
    ARG_ACCEPT_THREAD* pThreadArg = (ARG_ACCEPT_THREAD*)lpThreadParameter;
    CConnectionManager* pMan = pThreadArg->pMan;
    ROUTE*   pRoute = pThreadArg->pRoute;
    delete pThreadArg;
    pRoute->listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, 0, 0, WSA_FLAG_OVERLAPPED);

    if (pRoute->listenSocket >= 0)
    {
        SOCKADDR_IN LocalAddr;
        memset(&LocalAddr, 0, sizeof(SOCKADDR_IN));
        BOOL       b = TRUE;

        LocalAddr.sin_family = AF_INET;
        LocalAddr.sin_port = htons(pRoute->local_port);
        LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;//(DWORD)pMan.m_Set.dwIP; 
        setsockopt(pRoute->listenSocket, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&b, sizeof(b));

        if (SOCKET_ERROR != bind(pRoute->listenSocket, (LPSOCKADDR)&LocalAddr, sizeof(LocalAddr)))
        {
            if (!listen(pRoute->listenSocket, SOMAXCONN))
            {
                while (WAIT_OBJECT_0 != WaitForSingleObject(pMan->m_hTerminate, 0))
                {
                    SOCKET s = WSAAccept(pRoute->listenSocket, 0, 0, 0, 0);
                    if (s == INVALID_SOCKET)
                        continue;

                    // Disable send bufferring on the socket.  Setting SO_SNDBUF
                    // to 0 causes winsock to stop bufferring sends and perform
                    // sends directly from our buffers, thereby reducing CPU usage.
                    int zero = 0;
                    setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));

                    // send / receive timeout
                    //          setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&(pMan->m_Set.dwTimeoutCheckPeriod), sizeof(int));      
                    //          setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&(pMan->m_Set.dwTimeoutCheckPeriod), sizeof(int));      

                    CSocket* pSocket = pMan->m_SocketHolder.CreateSocket(s);
                    if (pSocket)
                    {
                        CConnection* pConn = pMan->m_ConnList.AddConnection(pSocket);
                        //_DBG_SOCKET            pSocket->SetConn(pConn);
                        if (pConn)
                        {
                            CreateIoCompletionPort((HANDLE)s, pMan->GetPort(), 0, 2);
                            //CLogWriter::CheckConnHeap("Accept", pConn);
                            //CLogWriter::write("%u \t Conn=%u s=%u Accepted Port=%u \tAccep\r\n",  GetTickCount(), pConn, s, wPort);
                            //CLogWriter::FixLine("Accept", pConn);
                            // begin first action
                            // TODO
                            // ConnectEx : https://docs.microsoft.com/en-us/windows/win32/api/mswsock/nc-mswsock-lpfn_connectex
                            // AcceptEx : https://docs.microsoft.com/en-us/windows/win32/api/mswsock/nf-mswsock-acceptex
                            //((IOM_IOCOMPPROC)((CConnection*)pConn)->m_pCurProc)(1, 0, pConn, 0, pMan, NULL);
                        }
                        else
                        {
                            CLogWriter::log(RET_NOTENOUGH_MEMORY);
                            pMan->m_SocketHolder.Release(pSocket);
                        }
                    }
                    else
                    {
                        CLogWriter::log(RET_NOTENOUGH_MEMORY);
                        closesocket(s);
                    }
                }
            }
        }
        else
            CLogWriter::log(RET_CANNOT_CREATE_ACCEPT_SOCKET);
    }
    pMan->LeaveThread();
    return 0;
}

DWORD CALLBACK CConnectionManager::IoCompProc(LPVOID lpThreadParameter)
{
    CConnectionManager& Man = *((CConnectionManager*)lpThreadParameter);

    InterlockedIncrement(&Man.m_dwConnThreadCount);
    CONNECTION* pCONN = 0;
    DWORD dwBytes = 0, dwError = 0;
    ULONG_PTR dwEvent = 0;
    InterlockedIncrement(&Man.m_dwActiveThreadCount);

    for (;;)
    {
        InterlockedDecrement((long*)&Man.m_dwActiveThreadCount);
        BOOL b = GetQueuedCompletionStatus(Man.GetPort(), &dwBytes, &dwEvent, (LPOVERLAPPED*)&pCONN, DEF_CM_MAXTHREADIDLE);
        InterlockedIncrement((long*)&Man.m_dwActiveThreadCount);
        if (dwEvent == IOM_BREAK_LOOP)
            break;

        if (b)//if no errors
        {   // check for equality of number of active threads to number of threads. If equal
            // the manager will start an extra thread.
            if (Man.m_dwActiveThreadCount >= Man.m_dwConnThreadCount)
            {
                Man.StartThread(IoCompProc, &Man);
            }
            dwError = 0; // this value is used as arg. of pConn->m_pCurProc
        }
        else
        {
            dwError = GetLastError();
            if (dwError == WAIT_TIMEOUT)
            { //  idle time limit for the thread exceeded
                if (Man.m_dwConnThreadCount > DEF_CM_MINTHREADCOUNT) {
                    printf("Thread timeout: %d\r\n", Man.m_dwConnThreadCount);
                    break;
                }
                continue;
            }
        }
        if (pCONN)
        {
            // udjust error codes
            if (dwError && pCONN->m_dwTransfer == dwBytes)
                dwError = 0;
            if (dwEvent == IOM_SOCKET_ERROR || !dwBytes)
                dwError = WSAENOTSOCK;
            if (!dwError)
            {
                if (pCONN->m_io_type == SEND)
                    ;//TODO Man.m_Statistics.ChangeStat(STAT_OUT_TRAFFIC, dwBytes);
                else
                    ;//TODO Man.m_Statistics.ChangeStat(STAT_IN_TRAFFIC, dwBytes);
                ;//TODO ((CRouteConn*)pCONN)->RouteData(dwBytes, dwError);
            }
        }
    }

    InterlockedDecrement((long*)&Man.m_dwActiveThreadCount);
    InterlockedDecrement((long*)&Man.m_dwConnThreadCount);
    Man.LeaveThread();
    return 0;
}

DWORD CALLBACK CConnectionManager::ThreadEntry(LPVOID lpThreadParameter)
{
    CConnectionManager* pMan = (CConnectionManager*)lpThreadParameter;
    InterlockedIncrement(&pMan->m_dwAllThreadCount);
    LPTHREAD_START_ROUTINE proc = pMan->ThreadProc;
    PVOID arg = pMan->lpThreadParameter;
    SetEvent(pMan->m_hThreadsStarted);
    proc(arg);
    return 0;
}

BOOL CConnectionManager::StartThread(LPTHREAD_START_ROUTINE proc, PVOID arg)
{
    ThreadProc = proc;
    lpThreadParameter = arg;
    ResetEvent(m_hThreadsStarted);
    QueueUserWorkItem(ThreadEntry, this, WT_EXECUTELONGFUNCTION);
    WaitForSingleObject(m_hThreadsStarted, INFINITE);
    return TRUE;
}

void CConnectionManager::LeaveThread()
{
    if (!InterlockedDecrement((long*)&m_dwAllThreadCount))
        SetEvent(m_hAllThreadsClosed);
}
