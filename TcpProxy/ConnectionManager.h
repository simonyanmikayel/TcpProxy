#pragma once

#include "Socket.h"
#include "LogWriter.h"
#include "ConnectionList.h"

enum
{
    IOM_BREAK_LOOP = 0xFFFF,
    IOM_DB_GETSETTINGS,
    IOM_SOCKET_ERROR
};

struct ROUTE
{
    SOCKET listenSocket;
    u_short local_port;
    u_short pair_port;
};

class CConnectionManager
{
public:
	CConnectionManager();
    ~CConnectionManager();
    void AddRoute(ROUTE* pRoute);
    HANDLE GetPort() { return m_hPort; }
    CSocketHolder m_SocketHolder;
    CConnectionList m_ConnList;

private:
	CRITICAL_SECTION   m_cs;
    HANDLE m_hThreadsStarted;
    HANDLE m_hTerminate;
    HANDLE m_hAllThreadsClosed;
    HANDLE m_hPort;
    long   m_dwAllThreadCount;
    long   m_dwConnThreadCount;
    long   m_dwActiveThreadCount;

    BOOL  StartThread(LPTHREAD_START_ROUTINE, PVOID);
    void  LeaveThread();
    LPTHREAD_START_ROUTINE ThreadProc;
    LPVOID lpThreadParameter;
    static DWORD CALLBACK ThreadEntry(LPVOID lpThreadParameter);
    // thread procedures
    static DWORD CALLBACK AcceptProc(LPVOID lpThreadParameter);
    static DWORD CALLBACK IoCompProc(LPVOID lpThreadParameter);
};

struct ARG_ACCEPT_THREAD
{
    CConnectionManager* pMan;
    ROUTE*         pRoute;
    ARG_ACCEPT_THREAD(CConnectionManager* pMan, ROUTE* pRoute)
    {
        this->pMan = pMan;
        this->pRoute = pRoute;
    }
};

