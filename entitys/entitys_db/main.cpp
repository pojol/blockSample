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
#include <core/event.h>

#include <network/acceptor.h>
#include <network/connector.h>

#include <luaProxy/luaProxy.h>
#include <dbProxy/mysqlProxy.h>
#include <distributed/node.h>
#include <timer/timer.h>

#include <log/log.h>
#include <iostream>
#include <stack>

class DBModuleCtl
	: public gsf::Module
	, public gsf::IEvent
{
public:
	DBModuleCtl()
		: Module("DBModuleCtl")
	{}

	void init() override
	{
		auto luaproxy_m_ = APP.getModule("LuaProxyModule");
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::makeArgs(getModuleID(), "entitys/db.lua"));
	}
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

	app.createModule(gsf::EventModule::get_ptr());
	app.createModule(new gsf::modules::LogModule());
	app.createModule(new gsf::modules::NodeModule);
	app.createModule(new gsf::network::AcceptorModule);
	app.createModule(new gsf::modules::MysqlProxyModule);
	app.createModule(new gsf::modules::LuaProxyModule);
	app.createModule(new gsf::modules::TimerModule);

	app.createModule(new DBModuleCtl);

	app.run();

	return 0;
}
