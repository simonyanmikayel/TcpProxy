// TcpProxy.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "Helpers.h"
#include "wsock.h"
#include "MainFrame.h"


CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	gMainFrame = new CMainFrame;

	if (gMainFrame->CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}
	gMainFrame->ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	delete gMainFrame;

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	//stdlog("hi\n");

	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	if (!wsock::Startup())
	{
		Helpers::SysErrMessageBox(TEXT("Failed to startup winsock"));
		return 1;
	}
	if (!wsock::InitExtensions())
	{
		Helpers::SysErrMessageBox(TEXT("Failed to initialize winsock.\nThis app requires Windows 8.1 and later."));
		wsock::Cleanup();
		return 1;
	}

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls
	//HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	//ATLASSERT(hInstRich != NULL);
	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	wsock::Cleanup();
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
