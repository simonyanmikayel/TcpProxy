#include "stdafx.h"
#include "wsock.h"

namespace wsock
{
	static boolean initialized = false;
	LPFN_ACCEPTEX lpfnAcceptEx = NULL;
	LPFN_CONNECTEX lpfnConnectEx = NULL;

	boolean Startup()
	{
		if (initialized)
			return true;

		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		initialized = (iResult == NO_ERROR);
		return initialized;
	}

	void Cleanup()
	{
		if (initialized)
		{
			WSACleanup();
			initialized = false;
		}
	}

	boolean InitExtensions()
	{
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		DWORD dwBytes;

		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		int iResultAcceptEx = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, sizeof(GuidAcceptEx),
			&lpfnAcceptEx, sizeof(lpfnAcceptEx),
			&dwBytes, NULL, NULL);

		GUID GuidConnectEx = WSAID_CONNECTEX;
		int iResultConnectEx = WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidConnectEx, sizeof(GuidConnectEx),
			&lpfnConnectEx, sizeof(lpfnConnectEx),
			&dwBytes, NULL, NULL);

		closesocket(s);
		return iResultAcceptEx != SOCKET_ERROR && iResultConnectEx != SOCKET_ERROR;
	}
};
