#pragma once

#include "Router.h"

class Proxy
{
public:
    Proxy();
    ~Proxy();
    boolean Start(const std::vector<ROUTE>& routes);
    void Stop();
    void removeConnection(Connection* pConnection, IO_ACTION action, ERROR_SOURCE error_source, const char* func, int line);
    bool StopRandomly(DWORD timeout);
    void AddRoute(const ROUTE& r);
#ifdef _SHOW_ALL_ROWTES
    void ShowRoutes();
#endif
    boolean Running() { return m_hIoCompPort != NULL; }
    HANDLE GetIoCompletionPort() { return m_hIoCompPort; }

private:
    HANDLE m_hIoCompPort = NULL;
    HANDLE m_hIoCompThread = NULL;
    HANDLE m_hThreadStarted = NULL;
    boolean m_bStoped;
    std::list<std::unique_ptr<Router>> m_Routers;
    static DWORD WINAPI IoCompThread(LPVOID lpParameter);
};

extern Proxy gProxy;