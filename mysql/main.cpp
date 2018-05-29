#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sstream>
#include <iostream>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <core/application.h>
#include <core/dynamic_module_factory.h>

#include <utils/timer.hpp>
#include <utils/logger.hpp>

#include <luaAdapter/luaAdapter.h>

#include "DBEntity.hpp"
#include <thread>

#include <DbgHelp.h>

//生产DUMP文件
int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, PWCHAR pwAppName)
{
	BOOL bOwnDumpFile = FALSE;
	HANDLE hDumpFile = hFile;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);

	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelp = LoadLibrary("DbgHelp.dll");
	if (hDbgHelp)
		pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	if (pfnMiniDumpWriteDump)
	{
		if (hDumpFile == NULL || hDumpFile == INVALID_HANDLE_VALUE)
		{
			//TCHAR szPath[MAX_PATH] = { 0 };
			TCHAR szFileName[MAX_PATH] = { 0 };
			//TCHAR* szAppName = pwAppName;
			TCHAR* szVersion = "v1.0";
			TCHAR dwBufferSize = MAX_PATH;
			SYSTEMTIME stLocalTime;

			GetLocalTime(&stLocalTime);
			//GetTempPath(dwBufferSize, szPath);

			//wsprintf(szFileName, L"%s%s", szPath, szAppName);
			CreateDirectory(szFileName, NULL);

			wsprintf(szFileName, "%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
				//szPath, szAppName, szVersion,
				szVersion,
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
				GetCurrentProcessId(), GetCurrentThreadId());
			hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

			bOwnDumpFile = TRUE;
			OutputDebugString(szFileName);
		}

		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			ExpParam.ThreadId = GetCurrentThreadId();
			ExpParam.ExceptionPointers = pExceptionPointers;
			ExpParam.ClientPointers = FALSE;

			pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
				hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);

			if (bOwnDumpFile)
				CloseHandle(hDumpFile);
		}
	}

	if (hDbgHelp != NULL)
		FreeLibrary(hDbgHelp);

	return EXCEPTION_EXECUTE_HANDLER;
}


LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return GenerateMiniDump(NULL, lpExceptionInfo, L"test");
}

class TestCaseLuaModule
	: public block::modules::LuaAdapterModule
{
public:
	TestCaseLuaModule()
		: block::modules::LuaAdapterModule("TestCaseLuaModule")
	{
		dir_ = "c:/github/blockSample/script";
		name_ = "test_mysql.lua";
	}

	virtual ~TestCaseLuaModule() {}

private:
	uint32_t luaproxy_m_ = 0;
};

class GetCharModule
	: public block::Module
{
public:
	GetCharModule()
		: block::Module("GetCharModule")
	{
	}

	void getChar()
	{
		while (true)
		{
			std::string line_ = "";

			getline(std::cin, line_, '\n');
			if (line_ == "reload") {
				dispatch(luaM_, eid::lua::reload, nullptr);
			}

			Sleep(10);
		}
	}

	void init() override
	{
		luaM_ = APP.getModule("TestCaseLuaModule");

		std::thread t1(&GetCharModule::getChar, this);
		t1.detach();
	}

private:
	block::ModuleID luaM_ = block::ModuleNil;
};


int main()
{
	block::Application app;
	block::AppConfig cfg;
	//cfg.is_watch_pref = true;
	app.initCfg(cfg);

	APP.createModule(new GetCharModule);
	APP.createModule(new TestCaseLuaModule);
	APP.createModule(new DBEntityModule);

	SetUnhandledExceptionFilter(ExceptionFilter);

	app.run();

	return 0;
}
