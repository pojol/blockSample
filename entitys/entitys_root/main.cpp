
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

#include <distributed/node.h>
#include <distributed/coordinate.h>

#include <luaAdapter/luaAdapter.h>

#include <log/log.h>


class RootModuleCtl
	: public gsf::modules::LuaAdapterModule
{
public:
	RootModuleCtl()
		: gsf::modules::LuaAdapterModule("RootModuleCtl")
	{
		dir_ = "C:/github/gsf_sample/script";
		name_ = "entitys/entitys/root.lua";
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
	cfg.name = "root";
	cfg.is_watch_pref = true;
	app.initCfg(cfg);

	app.createModule(new gsf::modules::LogModule);
	app.createModule(new gsf::network::AcceptorModule);
	app.createModule(new gsf::modules::CoodinatorModule);

	app.createModule(new RootModuleCtl);

	app.run();

	return 0;
}
