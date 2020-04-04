#pragma once

namespace wsock
{
	boolean Startup();
	boolean InitExtensions();
	void Cleanup();
	extern LPFN_ACCEPTEX lpfnAcceptEx;
	extern LPFN_CONNECTEX lpfnConnectEx;
};

