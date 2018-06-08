
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

#include <network\tcpAcceptor.h>
#include <network\tcpConnector.h>

#include <distributed\node.h>

#include <luaAdapter/luaAdapter.h>


class LoginNode
	: public block::modules::LuaAdapterModule
{
public:
	LoginNode()
		: block::modules::LuaAdapterModule("LoginNode")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "entitys/loginNode.lua";
	}

private:
};

class LoginModule
	: public block::modules::LuaAdapterModule
{
public:
	LoginModule()
		: block::modules::LuaAdapterModule("LoginModule")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "entitys/login.lua";
	}
};

int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	new block::Application;
	block::AppConfig cfg;
	cfg.name = "login";
	
	APP.initCfg(cfg);

	APP.createModule(new block::network::TcpAcceptorModule);
	APP.createModule(new block::modules::NodeModule);

	APP.createModule(new LoginModule);
	APP.createModule(new LoginNode);

	APP.run();

	return 0;
}