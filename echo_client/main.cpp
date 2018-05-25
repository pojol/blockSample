
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

#include <utils/logger.hpp>
#include <utils/timer.hpp>

#include <random>
#include <luaAdapter/luaAdapter.h>


class ClientModuleCtl
	: public block::modules::LuaAdapterModule
{
public:
	ClientModuleCtl()
		: block::modules::LuaAdapterModule("ClientModuleCtl")
	{}

	void before_init() override
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "echo/echo_client.lua";
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

	app.createModule(new ClientModuleCtl);

	app.run();

	return 0;
}
