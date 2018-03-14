
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
#include <core/event.h>
#include <core/dynamic_module_factory.h>

#include <network/acceptor.h>
#include <network/connector.h>
#include <timer/timer.h>

#include <log/log.h>

#include <random>
#include <luaProxy/luaProxy.h>


class ServerModuleCtl
	: public gsf::Module
	, public gsf::IEvent
{
public:
	ServerModuleCtl()
		: Module("ServerModuleCtl")
	{}

	void init() override
	{
		auto luaproxy_m_ = APP.getModule("LuaProxyModule");
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::makeArgs(getModuleID(), "echo/echo_server.lua"));
	}
};


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.name = "test_echo";
	app.initCfg(cfg);

	app.createModule(new gsf::modules::LogModule);
	app.createModule(new gsf::modules::LuaProxyModule);
	app.createModule(new gsf::network::ConnectorModule);
	app.createModule(new gsf::network::AcceptorModule);
	app.createModule(new gsf::modules::TimerModule);

	app.createModule(new ServerModuleCtl);

	app.run();

	return 0;
}