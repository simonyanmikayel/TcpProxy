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

enum class IO_TYPE { NONE, ACCEPT, CONNECT, RECV, SEND, LAST};
inline char* IoTypeNmae(IO_TYPE i) { 
    char* r = "?";  
    if (i == IO_TYPE::NONE) r = "IO_TYPE::NONE"; 
    else if (i == IO_TYPE::ACCEPT) r = "IO_TYPE::ACCEPT";
    else if (i == IO_TYPE::CONNECT) r = "IO_TYPE::CONNECT";
    else if (i == IO_TYPE::RECV) r = "IO_TYPE::RECV";
    else if (i == IO_TYPE::SEND) r = "IO_TYPE::SEND";
    return r; 
}

struct Socket : WSAOVERLAPPED
{
    Socket(Connection* pConnection) : m_pConnection(pConnection), m_s(INVALID_SOCKET), m_io_type(IO_TYPE::NONE), This(this){ ENTER_FUNC(); memset(this, 0, sizeof(WSAOVERLAPPED)); }
    ~Socket() { ENTER_FUNC(); close(); }
    void close() { ENTER_FUNC(); if (m_s != INVALID_SOCKET) { closesocket(m_s), m_s = INVALID_SOCKET; } }
    SOCKET m_s;
    IO_TYPE m_io_type;
    Connection* m_pConnection;
    void* This;
    static const int bufSize = 1024 * 128;
    char buf[bufSize];
};

struct Connection
{
    Connection(Router* pRouter) : m_pRouter(pRouter), m_AcceptSocket(this), m_ConnectSocket(this), err(0), m_io_type(IO_TYPE::NONE), m_id(++m_ID) { ENTER_FUNC(); }
    ~Connection() { ENTER_FUNC(); }
    void close() { ENTER_FUNC(); m_AcceptSocket.close(); m_ConnectSocket.close(); }
    SOCKET_TYPE SocketType(const Socket* pSocket) { return pSocket == &m_AcceptSocket ? SOCKET_TYPE::ACCEPT : SOCKET_TYPE::CONNECT; }
    Socket* GetPear(Socket* pSocket) { return pSocket == &m_AcceptSocket ? &m_ConnectSocket : &m_AcceptSocket; }
    boolean IsAccepSocket(Socket* pSocket) { return pSocket == &m_AcceptSocket; }
    boolean IsConnectSocket(Socket* pSocket) { return pSocket == &m_ConnectSocket; }
    DWORD ID() const { return m_id; }
    Router* m_pRouter;
    Socket m_AcceptSocket;
    Socket m_ConnectSocket;
    ULONG_PTR err;
    IO_TYPE m_io_type;
private:
    static DWORD m_ID;
    DWORD m_id;
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

private:
    static DWORD m_ID;
    DWORD m_id;
    ROUTE m_Route;
    SOCKET m_ListenSocket;
    std::list<std::unique_ptr<Connection>> m_connections;
};
