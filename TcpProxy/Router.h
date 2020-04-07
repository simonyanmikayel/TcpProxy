#pragma once

#include "ROUTE.h"

class Router;
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

struct Socket : WSAOVERLAPPED
{
    friend struct Connection;
    Socket(Connection* pConnection) : m_pConnection(pConnection), m_s(INVALID_SOCKET), m_io_action(IO_ACTION::NONE), This(this){ ENTER_FUNC(); memset(this, 0, sizeof(WSAOVERLAPPED)); }
    ~Socket() { ENTER_FUNC(); CloseSocket(); }
    SOCKET m_s;
    IO_ACTION m_io_action;
    Connection* m_pConnection;
    void* This;
    static const int bufSize = 1024 * 128;
    char buf[bufSize];
private:
    void CloseSocket() { ENTER_FUNC(); if (m_s != INVALID_SOCKET) { closesocket(m_s), m_s = INVALID_SOCKET; } }
};

struct Connection
{
    Connection(Router* pRouter) : m_pRouter(pRouter), m_AcceptSocket(this), m_ConnectSocket(this) { ENTER_FUNC(); GetLocalTime(&initTime); }
    ~Connection() { ENTER_FUNC(); m_AcceptSocket.CloseSocket(); m_ConnectSocket.CloseSocket(); }
    void close(IO_ACTION action) { onClose(action); };
    void onClose(IO_ACTION action);
    void onConnect();
    void onRecv();
    SOCKET_TYPE SocketType(const Socket* pSocket) { return pSocket == &m_AcceptSocket ? SOCKET_TYPE::ACCEPT : SOCKET_TYPE::CONNECT; }
    Socket* GetPear(Socket* pSocket) { return pSocket == &m_AcceptSocket ? &m_ConnectSocket : &m_AcceptSocket; }
    boolean IsAccepSocket(Socket* pSocket) { return pSocket == &m_AcceptSocket; }
    boolean IsConnectSocket(Socket* pSocket) { return pSocket == &m_ConnectSocket; }
    DWORD ID() const { return m_id; }
    Router* m_pRouter;
    Socket m_AcceptSocket;
    Socket m_ConnectSocket;
    ULONG_PTR m_err = 0;
    IO_ACTION m_io_action = IO_ACTION::NONE;
    SYSTEMTIME initTime = { 0 };
    SYSTEMTIME connectTime = {0};
    SYSTEMTIME closeTime = { 0 };
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
    boolean DoRoute(Socket* pRecvSocket, DWORD dwNumberOfBytes, HANDLE hIoCompPort);
    DWORD ID() const { return m_id; }
    void Stop();
    const ROUTE* GetRote() const { return &m_Route; }

private:
    static DWORD m_ID;
    DWORD m_id;
    ROUTE m_Route;
    SOCKET m_ListenSocket;
    std::list<std::unique_ptr<Connection>> m_connections;
};
