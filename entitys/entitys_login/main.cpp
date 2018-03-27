
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

#include <network/acceptor.h>
#include <network/connector.h>

#include <log/log.h>
#include <luaAdapter/luaAdapter.h>
#include <distributed/node.h>
#include <timer/timer.h>


class LoginModuleCtl
	: public gsf::modules::LuaAdapterModule
{
public:
	LoginModuleCtl()
		: gsf::modules::LuaAdapterModule("LoginModuleCtl")
	{
		dir_ = "C:/github/gsf_sample/script";
		name_ = "entitys/login.lua";
	}

private:
};

int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.name = "login";
	app.initCfg(cfg);

	app.createModule(new gsf::modules::LogModule);
	app.createModule(new gsf::network::AcceptorModule);
	app.createModule(new gsf::modules::TimerModule);
	app.createModule(new gsf::modules::NodeModule);

	app.createModule(new LoginModuleCtl);

	app.run();

	return 0;
}