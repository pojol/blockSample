
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

#include <network/tcpAcceptor.h>
#include <network/tcpConnector.h>

#include <utils/timer.hpp>
#include <utils/logger.hpp>

#include <random>
#include <luaAdapter/luaAdapter.h>


class ServerModuleCtl
	: public block::modules::LuaAdapterModule
{
public:
	ServerModuleCtl()
		: block::modules::LuaAdapterModule("ServerModuleCtl")
	{}

	void before_init() override
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "echo/echo_server.lua";
	}
};


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	block::Application app;
	block::AppConfig cfg;
	cfg.name = "test_echo";
	app.initCfg(cfg);

	app.createModule(new block::network::TcpConnectorModule);
	app.createModule(new block::network::TcpAcceptorModule);

	app.createModule(new ServerModuleCtl);

	app.run();

	return 0;
}
