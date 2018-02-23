
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

#include <distributed/node.h>
#include <distributed/coordinate.h>

#include <lua_proxy/lua_proxy.h>

#include <log/log.h>

class RootModuleCtl
	: public gsf::Module
	, public gsf::IEvent
{
public:
	RootModuleCtl()
		: Module("RootModuleCtl")
	{}

	void init() override
	{
		auto luaproxy_m_ = APP.get_module("LuaProxyModule");
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), "entitys/root.lua"));
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
	cfg.name = "root";
	cfg.is_watch_pref = true;
	app.init_cfg(cfg);

	app.create_module(new gsf::modules::LogModule);
	app.create_module(new gsf::modules::LuaProxyModule);
	app.create_module(new gsf::network::AcceptorModule);
	app.create_module(new gsf::modules::CoodinatorModule);

	app.create_module(new RootModuleCtl);

	app.run();

	return 0;
}
