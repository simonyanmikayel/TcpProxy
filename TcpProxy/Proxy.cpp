#include "stdafx.h"
#include "Helpers.h"
#include "Archive.h"
#include "Proxy.h"

Proxy gProxy;

Proxy::Proxy()
{
	ENTER_FUNC();
	m_bStoped = true;
	m_hThreadStarted = CreateEvent(0, TRUE, FALSE, 0);
}

Proxy::~Proxy()
{
	ENTER_FUNC();
	Stop();
}

boolean Proxy::Start(const std::vector<ROUTE>& routes)
{
	ENTER_FUNC();
	if (Running())
		return true;
	m_hIoCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long)0, 0);

	ResetEvent(m_hThreadStarted);
	DWORD dwTID;
	m_bStoped = false;
	m_hIoCompThread = CreateThread(0, 0, IoCompThread, this, 0, &dwTID);
	WaitForSingleObject(m_hThreadStarted, INFINITE);

	for (auto& r : routes)
	{
		AddRoute(r);
	}
	return true;
}

void Proxy::Stop()
{
	ENTER_FUNC();
	if (!Running())
		return;

	m_bStoped = true;
	m_Routers.clear();
	CloseHandle(m_hIoCompPort);
	WaitForSingleObject(m_hIoCompThread, INFINITE);
	m_hIoCompPort = NULL;
	m_hIoCompThread = NULL;
}

void Proxy::AddRoute(const ROUTE& r)
{
	ENTER_FUNC();
	m_Routers.push_back(std::make_unique<Router>(r));
	std::unique_ptr<Router>& router = m_Routers.back();
	gArchive.addRouter(router.get());
	if (Running())
		router->StartListening(m_hIoCompPort);
}

void Proxy::ShowRoutes()
{
	for (auto& r : m_Routers)
	{
		gArchive.addRouter(r.get());
	}
}

DWORD WINAPI Proxy::IoCompThread(LPVOID lpParameter)
{
	ENTER_FUNC();
	Proxy* pProxy = reinterpret_cast<Proxy*>(lpParameter);
	SetEvent(pProxy->m_hThreadStarted);

	for (;;)
	{
		//https://docs.microsoft.com/en-us/windows/win32/fileio/getqueuedcompletionstatusex-func
		OVERLAPPED_ENTRY overlapped_entries[32];
		ULONG ulNumEntriesRemoved = 0;
		BOOL ok = GetQueuedCompletionStatusEx(pProxy->m_hIoCompPort, overlapped_entries, _countof(overlapped_entries), &ulNumEntriesRemoved, INFINITE, FALSE);
		if (pProxy->m_bStoped || !ok)
		{
			if (!pProxy->m_bStoped)
				Helpers::SysErrMessageBox("GetQueuedCompletionStatusEx failed with error: %u", GetLastError());
			break;
		}
		if (ulNumEntriesRemoved <= 0)
		{
			STDLOG("ulNumEntriesRemoved <= 0 ???");
			continue;
		}
		for (ULONG i = 0; i < ulNumEntriesRemoved; i++)
		{
			OVERLAPPED_ENTRY& entry = overlapped_entries[i];
			Socket* pSocket = (Socket*)entry.lpOverlapped;
			Connection* pConnection = pSocket->m_pConnection;
			if (pSocket->This != (void*)pSocket || pSocket->m_io_action <= IO_ACTION::NONE || pSocket->m_io_action >= IO_ACTION::PROXY_STOP || (!pConnection->IsAccepSocket(pSocket) && !pConnection->IsConnectSocket(pSocket)) )
			{
				Helpers::SysErrMessageBox("Implementation error 1");
				break;
			}
			else
			{
				ULONG_PTR err = pSocket->Internal;
				Router* pRouter = pConnection->m_pRouter;
				pConnection->m_err = err;

				STDLOG("pSocket: %s m_io_action=%s  bytes=%d err=%X", 
					SocketTypeNmae(pConnection->SocketType(pSocket)), 
					IoTypeNmae(pSocket->m_io_action), 
					entry.dwNumberOfBytesTransferred, err);

				if (IO_ACTION::ACCEPT == pSocket->m_io_action)
				{
					if (!pRouter->DoAccept(pProxy->m_hIoCompPort)) //loop of accept
						pConnection->close(IO_ACTION::ACCEPT);
				}
				if (err == 0 && entry.dwNumberOfBytesTransferred == 0 && (IO_ACTION::RECV == pSocket->m_io_action || IO_ACTION::SEND == pSocket->m_io_action))
				{
					err = SOCKET_ERROR;
				}
				if (err == 0)
				{
					if (IO_ACTION::ACCEPT == pSocket->m_io_action)
					{
						gArchive.addConnection(pConnection);
						if (!pRouter->DoConnect(pConnection, pProxy->m_hIoCompPort))
							pConnection->close(IO_ACTION::CONNECT);
					}
					else if (IO_ACTION::CONNECT == pSocket->m_io_action)
					{
						pConnection->onConnect();
						if (!pRouter->DoRecv(&pConnection->m_AcceptSocket, pProxy->m_hIoCompPort) ||
							!pRouter->DoRecv(&pConnection->m_ConnectSocket, pProxy->m_hIoCompPort))
							pConnection->close(IO_ACTION::RECV);
					}
					else if (IO_ACTION::RECV == pSocket->m_io_action)
					{
						gArchive.addRecv(pSocket, pSocket->buf, entry.dwNumberOfBytesTransferred);
						pConnection->onRecv();
						if (!pRouter->DoRoute(pSocket, entry.dwNumberOfBytesTransferred, pProxy->m_hIoCompPort) ||
							!pRouter->DoRecv(pSocket, pProxy->m_hIoCompPort)) // recive loop
							pConnection->close(IO_ACTION::RECV);
					}
					else if (IO_ACTION::SEND == pSocket->m_io_action)
					{
						if (!pRouter->DoRecv(pSocket, pProxy->m_hIoCompPort)) // recive loop
							pConnection->close(IO_ACTION::SEND);
					}
				}
				else
				{
					if (err != STATUS_PENDING)
						pConnection->close(pSocket->m_io_action);
				}
			}
		}
	}

	EXIT_FUNC();
	return 0;
}
