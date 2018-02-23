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

#include <lua_proxy/lua_proxy.h>
#include <mysql_proxy/mysql_proxy.h>
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
		auto luaproxy_m_ = APP.get_module("LuaProxyModule");
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), "entitys/db.lua"));
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
	app.init_cfg(cfg);

	app.create_module(gsf::EventModule::get_ptr());
	app.create_module(new gsf::modules::LogModule());
	app.create_module(new gsf::modules::NodeModule);
	app.create_module(new gsf::network::AcceptorModule);
	app.create_module(new gsf::modules::MysqlProxyModule);
	app.create_module(new gsf::modules::LuaProxyModule);
	app.create_module(new gsf::modules::TimerModule);

	app.create_module(new DBModuleCtl);

	app.run();

	return 0;
}
