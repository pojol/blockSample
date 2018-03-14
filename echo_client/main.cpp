
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
#include <lua_proxy/lua_proxy.h>


class ClientModuleCtl
	: public gsf::Module
	, public gsf::IEvent
{
public:
	ClientModuleCtl()
		: Module("ClientModuleCtl")
	{}

	void init() override
	{
		auto luaproxy_m_ = APP.get_module("LuaProxyModule");
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), "echo/echo_client.lua"));
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
	app.init_cfg(cfg);

	app.create_module(new gsf::modules::LogModule);
	app.create_module(new gsf::modules::LuaProxyModule);
	app.create_module(new gsf::network::ConnectorModule);
	app.create_module(new gsf::network::AcceptorModule);
	app.create_module(new gsf::modules::TimerModule);

	app.create_module(new ClientModuleCtl);

	app.run();

	return 0;
}
