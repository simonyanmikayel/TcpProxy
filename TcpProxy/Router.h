#pragma once

#include "ROUTE.h"

class Router;
struct Socket;
struct Connection;

enum class SOCKET_TYPE { ACCEPT, CONNECT };
inline char* SocketTypeNmae(SOCKET_TYPE i) {
    char* r = "?";
    if (i == SOCKET_TYPE::ACCEPT) r = "LOCAL";
    else if (i == SOCKET_TYPE::CONNECT) r = "REMOTE";
    return r;
}

enum class IO_ACTION { NONE, ACCEPT, CONNECT, RECV, SEND, PROXY_STOP };
inline char* IoTypeNmae(IO_ACTION i) { 
    char* r = "?";  
    if (i == IO_ACTION::NONE) r = "IO_ACTION::NONE"; 
    else if (i == IO_ACTION::ACCEPT) r = "IO_ACTION::ACCEPT";
    else if (i == IO_ACTION::CONNECT) r = "IO_ACTION::CONNECT";
    else if (i == IO_ACTION::RECV) r = "IO_ACTION::RECV";
    else if (i == IO_ACTION::SEND) r = "IO_ACTION::SEND";
    return r; 
}

struct MYOVERLAPPED : OVERLAPPED
{
    MYOVERLAPPED(IO_ACTION action, Socket* pSocket) : m_io_action(action), m_pSocket(pSocket) {}
    void Init() { memset(this, 0, sizeof(WSAOVERLAPPED)); }
    Socket* GetSocket() { return m_pSocket; }
    IO_ACTION GetAction() { return m_io_action; }
private:
    IO_ACTION m_io_action;
    Socket* m_pSocket;
};

struct Socket
{
    friend struct Connection;
    Socket(Connection* pConnection) : m_pConnection(pConnection), m_s(INVALID_SOCKET), 
        m_ovlAccept(IO_ACTION::ACCEPT, this), 
        m_ovlConnect(IO_ACTION::CONNECT, this), 
        m_ovlRecv(IO_ACTION::RECV, this), 
        m_ovlSend(IO_ACTION::SEND, this),
        m_ovlError(IO_ACTION::PROXY_STOP, this),
        This(this)
    { 
        STDLOG(""); 
        memset(addrBuf, 0, sizeof(addrBuf));
    }
    ~Socket() { STDLOG(""); CloseSocket(); }
    OVERLAPPED* GetOverlapped(IO_ACTION action);
    SOCKET m_s;
    Connection* m_pConnection;
    void* This;
    static const size_t bufSize = 1024 * 128;
    char buf[bufSize];
    static const int addrBufLen = sizeof(sockaddr_in) + 16;
    char addrBuf[2 * addrBufLen];
private:
    MYOVERLAPPED m_ovlAccept, m_ovlConnect, m_ovlRecv, m_ovlSend, m_ovlError;
    void CloseSocket() { STDLOG(""); if (m_s != INVALID_SOCKET) { closesocket(m_s), m_s = INVALID_SOCKET; } }
};

struct Connection
{
    Connection(Router* pRouter) : m_pRouter(pRouter), m_AcceptSocket(this), m_ConnectSocket(this) { 
        STDLOG(""); GetLocalTime(&initTime); 
    }
    ~Connection() { STDLOG(""); m_AcceptSocket.CloseSocket(); m_ConnectSocket.CloseSocket(); }
    void close(IO_ACTION action) { onClose(action); };
    void onClose(IO_ACTION action);
    void onConnect();
    void onRecv();
    SOCKET_TYPE SocketType(const Socket* pSocket) const { return pSocket == &m_AcceptSocket ? SOCKET_TYPE::ACCEPT : SOCKET_TYPE::CONNECT; }
    Socket* GetPear(const Socket* pSocket) { return pSocket == &m_AcceptSocket ? &m_ConnectSocket : &m_AcceptSocket; }
    boolean IsAccepSocket(const Socket* pSocket) const { return pSocket == &m_AcceptSocket; }
    boolean IsConnectSocket(const Socket* pSocket) const { return pSocket == &m_ConnectSocket; }
    boolean IsOpen() { return opened && !closed; };
    DWORD ID() const { return m_id; }
    Router* m_pRouter;
    Socket m_AcceptSocket;
    Socket m_ConnectSocket;
    ULONG_PTR m_err = 0;
    IO_ACTION m_io_action = IO_ACTION::NONE;
    SYSTEMTIME initTime = { 0 };
    SYSTEMTIME connectTime = {0};
    SYSTEMTIME closeTime = { 0 };
    DWORD      closeTimeout = INFINITE;
private:
    boolean opened = false;
    boolean closed = false;
    static DWORD m_ID;
    DWORD m_id = (++m_ID);
};

class Router
{
public:
    Router(const ROUTE& r);
    virtual ~Router();
    boolean StartListening(HANDLE hIoCompPort);
    boolean DoAccept(HANDLE hIoCompPort);
    boolean DoConnect(Connection* pConnection, HANDLE hIoCompPort);
    boolean DoRecv(Socket* pSocket, HANDLE hIoCompPort);
    boolean DoSend(Socket* pSocket, DWORD dwNumberOfBytes, char* buf, HANDLE hIoCompPort);
    DWORD ID() const { return m_id; }
    void Stop();
    const ROUTE* GetRote() const { return &m_Route; }
    const std::list<std::unique_ptr<Connection>>& getConnections() { return m_connections; }

private:
    static DWORD m_ID;
    DWORD m_id;
    ROUTE m_Route;
    SOCKET m_ListenSocket;
    std::list<std::unique_ptr<Connection>> m_connections;
};
