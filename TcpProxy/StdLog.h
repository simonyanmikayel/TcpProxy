#pragma once

#ifdef _DEBUG
#define _STDLOG
#endif

void stdlog(const char* lpFormat, ...);
void Trace(const char* pszFormat, ...);
void stdclear();
#define STDLOG(fmt, ...) { stdlog( "%s %d :"fmt"\n", __FUNCTION__, __LINE__,  __VA_ARGS__); }
#define ENTER_FUNC() { stdlog( "-> %s\n", __FUNCTION__); }
#define EXIT_FUNC() { stdlog( "<-- %s\n", __FUNCTION__); }


