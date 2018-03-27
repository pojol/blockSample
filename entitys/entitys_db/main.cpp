#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>
#include <fmt/format.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <core/application.h>

#include <network/acceptor.h>
#include <network/connector.h>

#include <luaAdapter/luaAdapter.h>
#include <dbProxy/mysqlProxy.h>
#include <distributed/node.h>
#include <timer/timer.h>

#include <log/log.h>
#include <iostream>
#include <stack>

class DBModuleCtl
	: public gsf::modules::LuaAdapterModule
{
public:
	DBModuleCtl()
		: gsf::modules::LuaAdapterModule("DBModuleCtl")
	{
		dir_ = "C:/github/gsf_sample/script";
		name_ = "entitys/db.lua";
	}

private:
};

int main()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1, 1);
	int result = WSAStartup(wVersionRequested, &wsaData);
	if (result != 0) {
		exit(1);
	}
#endif // WIN32

	gsf::Application app;
	gsf::AppConfig cfg;
	app.initCfg(cfg);

	app.createModule(new gsf::modules::LogModule());
	app.createModule(new gsf::modules::NodeModule);
	app.createModule(new gsf::network::AcceptorModule);
	app.createModule(new gsf::modules::MysqlProxyModule);
	app.createModule(new gsf::modules::TimerModule);

	app.createModule(new DBModuleCtl);

	app.run();

	return 0;
}
