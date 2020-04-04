#pragma once

#include "Router.h"

class Proxy
{
public:
    Proxy();
    ~Proxy();
    boolean Start();
    void Stop();
    void AddRoute(const ROUTE& r);
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