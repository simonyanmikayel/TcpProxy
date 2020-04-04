#pragma once

class Proxy;
class Router;
struct Connection;

struct ROUTE
{
    ROUTE();
    ~ROUTE();
    u_short m_local_port = 0;
    u_short m_remoute_port = 0;
    std::string m_remoute_addr;
};

enum SOCKET_TYPE { S_ACCEPT, S_CONNECT };
inline char* SocketTypeNmae(SOCKET_TYPE i) {
    char* r = "?";
    if (i == S_ACCEPT) r = "LOCAL";
    else if (i == S_CONNECT) r = "REMOTE";
    return r;
}

enum IO_TYPE { IO_NONE, IO_ACCEPT, IO_CONNECT, IO_RECV, IO_SEND, IO_LAST};
inline char* IoTypeNmae(IO_TYPE i) { 
    char* r = "?";  
    if (i == IO_NONE) r = "IO_NONE"; 
    else if (i == IO_ACCEPT) r = "IO_ACCEPT";
    else if (i == IO_CONNECT) r = "IO_CONNECT";
    else if (i == IO_RECV) r = "IO_RECV";
    else if (i == IO_SEND) r = "IO_SEND";
    return r; 
}

struct Socket : WSAOVERLAPPED
{
    Socket(Connection* pConnection) : m_pConnection(pConnection), m_s(INVALID_SOCKET), m_io_type(IO_NONE), This(this){ ENTER_FUNC(); memset(this, 0, sizeof(WSAOVERLAPPED)); }
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
    Connection(Router* pRouter) : m_pRouter(pRouter), m_AcceptSocket(this), m_ConnectSocket(this), err(0) { ENTER_FUNC(); }
    ~Connection() { ENTER_FUNC(); }
    void close() { ENTER_FUNC(); m_AcceptSocket.close(); m_ConnectSocket.close(); }
    SOCKET_TYPE SocketType(Socket* pSocket) { return pSocket == &m_AcceptSocket ? S_ACCEPT : S_CONNECT; }
    Socket* GetPear(Socket* pSocket) { return pSocket == &m_AcceptSocket ? &m_ConnectSocket : &m_AcceptSocket; }
    boolean IsAccepSocket(Socket* pSocket) { return pSocket == &m_AcceptSocket; }
    boolean IsConnectSocket(Socket* pSocket) { return pSocket == &m_ConnectSocket; }
    Router* m_pRouter;
    Socket m_AcceptSocket;
    Socket m_ConnectSocket;
    ULONG_PTR err;
    IO_TYPE m_io_type;
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
    void Stop();

private:
    ROUTE m_Route;
    SOCKET m_ListenSocket;
    std::list<std::unique_ptr<Connection>> m_connections;
};
