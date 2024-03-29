#pragma once

#ifndef _DEBUG
#undef _STDLOG
#endif

void stdlog(const char* lpFormat, ...);
void Trace(const char* pszFormat, ...);
void stdclear();
#define STDLOG(fmt, ...) { stdlog( "%s %d :"#fmt"\n", __FUNCTION__, __LINE__,  __VA_ARGS__); }


