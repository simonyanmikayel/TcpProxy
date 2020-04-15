#include "stdafx.h"
#include "Helpers.h"
#include "wsock.h"
#include "Router.h"
#include "Archive.h"

//https://docs.microsoft.com/en-us/windows/win32/api/mswsock/nf-mswsock-acceptex

DWORD Router::m_ID = 0;
DWORD Connection::m_ID = 0;

OVERLAPPED* Socket::GetOverlapped(IO_ACTION action)
{
	MYOVERLAPPED* p = nullptr;
	if (action == IO_ACTION::ACCEPT)
		p = &m_ovlAccept;
	else if (action == IO_ACTION::CONNECT)
		p = &m_ovlConnect;
	else if (action == IO_ACTION::RECV)
		p = &m_ovlRecv;
	else if (action == IO_ACTION::SEND)
		p = &m_ovlSend;
	else
		p = &m_ovlError;
	p->Init();
	return p;
}

void Connection::onConnect()
{
	opened = true;
	GetLocalTime(&connectTime);
	SOCKADDR_IN* localAddr = (SOCKADDR_IN*)m_AcceptSocket.addrBuf;
	SOCKADDR_IN* remoteAddr = (SOCKADDR_IN*)(m_AcceptSocket.addrBuf + Socket::addrBufLen);
	char* localName = inet_ntoa(localAddr->sin_addr);
	char* remoteName = inet_ntoa(remoteAddr->sin_addr);
	CONN_NODE* pNode = gArchive.getConnection(this);
	if (pNode)
	{
		pNode->connectTime = connectTime;
		pNode->opened = 1;
		if (remoteName)
			strncpy_s(pNode->peername, remoteName, _countof(pNode->peername) - 1);
		if (pNode->posInTree && pNode->parent && pNode->parent->expanded)
			::PostMessage(hwndMain, WM_UPDATE_TREE, (WPARAM)pNode->posInTree, (LPARAM)pNode->posInTree);
	}
}

void Connection::onRecv()
{
	CONN_NODE* pNode = gArchive.getConnection(this);
	if (pNode)
	{
		if (pNode->posInTree && pNode->parent && pNode->parent->expanded)
			::PostMessage(hwndMain, WM_UPDATE_TREE, (WPARAM)pNode->posInTree, (LPARAM)pNode->posInTree);
	}
}

void Connection::onClose(IO_ACTION action)
{ 
	STDLOG(""); 
	if (!closed)
	{
		closed = true;
		m_io_action = action;
		m_AcceptSocket.CloseSocket();
		m_ConnectSocket.CloseSocket();
		GetLocalTime(&closeTime);
		CONN_NODE* pNode = gArchive.getConnection(this);
		if (pNode)
		{
			pNode->closed = 1;
			pNode->action = action;
			pNode->closeTime = closeTime;
			if (pNode->posInTree && pNode->parent && pNode->parent->expanded)
				::PostMessage(hwndMain, WM_UPDATE_TREE, (WPARAM)pNode->posInTree, (LPARAM)pNode->posInTree);
		}
	}
}

Router::Router(const ROUTE& r) :
	m_Route{r}
	, m_ListenSocket(INVALID_SOCKET)
	, m_id(++m_ID)
{
	STDLOG("");
}

Router::~Router()
{
	STDLOG("");
	Stop();
}

SOCKET CreateBoundTcpSocket(u_short port = 0)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s != INVALID_SOCKET)
	{
		// Bind the listening socket to the local IP address and port
		SOCKADDR_IN LocalAddr;
		memset(&LocalAddr, 0, sizeof(SOCKADDR_IN));
		DWORD       b = TRUE;
		LocalAddr.sin_family = AF_INET;
		LocalAddr.sin_port = htons(port);
		LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&b, sizeof(b));
		if (SOCKET_ERROR == bind(s, (LPSOCKADDR)&LocalAddr, sizeof(LocalAddr)))
		{
			closesocket(s);
			s = INVALID_SOCKET;
		}
	}
	return s;
}

boolean Router::StartListening(HANDLE hIoCompPort)
{
	STDLOG("");
	if (!m_Route.IsValid())
		return false;
	// Create a listening socket
	m_ListenSocket = CreateBoundTcpSocket(m_Route.local_port);
	if (m_ListenSocket == INVALID_SOCKET) 
	{
		Helpers::SysErrMessageBox("Create of ListenSocket socket failed with error: %u\n", WSAGetLastError());
		return false;
	}

	DWORD b = 1;
	setsockopt(m_ListenSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (LPCSTR)&b, sizeof(b));

	// Associate the listening socket with the completion port
	HANDLE hCompPort2 = CreateIoCompletionPort((HANDLE)m_ListenSocket, hIoCompPort, (u_long)0, 0);
	if (hCompPort2 == NULL) {
		Helpers::SysErrMessageBox("StartListening: CreateIoCompletionPort associate failed with error: %u\n", GetLastError());
		return false;
	}

	if (SOCKET_ERROR == listen(m_ListenSocket, SOMAXCONN))
	{
		Helpers::SysErrMessageBox("Listen of ListenSocket socket failed with error: %u\n", WSAGetLastError());
		closesocket(m_ListenSocket);
		return false;
	}

	// do first accept
	return DoAccept(hIoCompPort);
}

boolean Router::DoAccept(HANDLE hIoCompPort)
{
	// https://docs.microsoft.com/en-us/windows/win32/api/mswsock/nf-mswsock-acceptex
	STDLOG("");
	m_connections.push_back(std::make_unique<Connection>(this));
	Socket& AcceptSocket = m_connections.back()->m_AcceptSocket;

	AcceptSocket.m_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (AcceptSocket.m_s == INVALID_SOCKET) 
	{
		Helpers::SysErrMessageBox("Create accept socket failed with error: %u\n", WSAGetLastError());
		return false;
	}	

	DWORD b = 1;
	setsockopt(AcceptSocket.m_s, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (LPCSTR)&b, sizeof(b));

	// Associate the accept socket with the completion port
	HANDLE hCompPort2 = CreateIoCompletionPort((HANDLE)AcceptSocket.m_s, hIoCompPort, (u_long)0, 0);
	if (hCompPort2 == NULL) 
	{
		Helpers::SysErrMessageBox("DoAccept: CreateIoCompletionPort associate failed with error: %u\n", GetLastError());
		return false;
	}

	DWORD dwBytes = 0;
	BOOL bRetVal = wsock::lpfnAcceptEx(
		m_ListenSocket, 
		AcceptSocket.m_s,
		AcceptSocket.addrBuf, //A pointer to a buffer that receives the first block of data sent on a new connection, the local address of the server, and the remote address of the client
		0, // If dwReceiveDataLength is zero, accepting the connection will not result in a receive operation. Instead, AcceptEx completes as soon as a connection arrives, without waiting for any data
		Socket::addrBufLen, //The number of bytes reserved for the local address information. This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
		Socket::addrBufLen, //The number of bytes reserved for the remote address information. This value must be at least 16 bytes more than the maximum address length for the transport protocol in use.
		&dwBytes, AcceptSocket.GetOverlapped(IO_ACTION::ACCEPT));

	if (bRetVal == FALSE)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != ERROR_IO_PENDING)
		{
			STDLOG("DoAccept error: %u", dwError);
			return false;
		}
	}
	return true;
}

boolean Router::DoConnect(Connection* pConnection, HANDLE hIoCompPort)
{
	//https://docs.microsoft.com/en-us/windows/win32/api/mswsock/nc-mswsock-lpfn_connectex
	STDLOG("");
	Socket& ConnectSocket = pConnection->m_ConnectSocket;
	ConnectSocket.m_s = CreateBoundTcpSocket(0);
	if (ConnectSocket.m_s == INVALID_SOCKET) 
	{
		Helpers::SysErrMessageBox("Create connect socket failed with error: %u\n", WSAGetLastError());
		return false;
	}

	// Associate the accept socket with the completion port
	HANDLE hCompPort2 = CreateIoCompletionPort((HANDLE)ConnectSocket.m_s, hIoCompPort, (u_long)0, 0);
	if (hCompPort2 == NULL) 
	{
		Helpers::SysErrMessageBox("DoConnect: CreateIoCompletionPort associate failed with error: %u\n", GetLastError());
		return false;
	}

	SOCKADDR_IN  server;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(m_Route.remote_port);
	server.sin_addr.s_addr = inet_addr(m_Route.remote_addr.c_str());
	STDLOG("Connecting to: %s:%d\n", inet_ntoa(server.sin_addr), m_Route.remote_port);
	DWORD dwBytes = 0;
	BOOL bRetVal = wsock::lpfnConnectEx(
		ConnectSocket.m_s,
		(SOCKADDR*)&server, 
		sizeof(server),
		NULL,
		0,
		&dwBytes,
		ConnectSocket.GetOverlapped(IO_ACTION::CONNECT));

	if (bRetVal == FALSE) 
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != ERROR_IO_PENDING)
		{
			STDLOG("DoConnect error: %u", dwError);
			return false;
		}
	}
	return true;
}

boolean Router::DoRecv(Socket* pSocket, HANDLE hIoCompPort)
{
	STDLOG("Socket %d", pSocket->m_s);
	DWORD dwBytes = 0, dwFlags = 0;
	WSABUF wsabuf = { pSocket->bufSize, pSocket->buf };
	int Result = WSARecv(pSocket->m_s, &wsabuf, 1, &dwBytes, &dwFlags, pSocket->GetOverlapped(IO_ACTION::RECV), 0);

	if (Result == SOCKET_ERROR) 
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != ERROR_IO_PENDING)
		{
			STDLOG("DoRecv error: %u", dwError);
			return false;
		}
	}
	return true;
}

boolean Router::DoSend(Socket* pSocket, DWORD dwNumberOfBytes, char* buf, HANDLE hIoCompPort)
{
	STDLOG("Socket %d dwNumberOfBytes-%d buf=%p", pSocket->m_s, dwNumberOfBytes, buf);
	DWORD dwBytes = 0, dwFlags = 0;
	WSABUF wsabuf = { dwNumberOfBytes, buf };
	int Result = WSASend(pSocket->m_s, &wsabuf, 1, &dwBytes, dwFlags, pSocket->GetOverlapped(IO_ACTION::SEND), 0);

	if (Result == SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();
		if (dwError != ERROR_IO_PENDING)
		{
			STDLOG("DoSend error: %u", dwError);
			return false;
		}
	}
	return true;
}

void Router::Stop()
{
	STDLOG("");
	closesocket(m_ListenSocket);
	for (auto& p : m_connections)
	{
		p->close(IO_ACTION::PROXY_STOP);
	}
}
