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

class RoomModule
	: public block::modules::LuaAdapterModule
{
public:
	RoomModule()
		: block::modules::LuaAdapterModule("RoomModule")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "room.lua";
	}
};

namespace block
{
	namespace modules 
	{
		REGISTER_CLASS(RoomModule)
	}

	namespace network
	{
		REGISTER_CLASS(TcpAcceptorModule)
	}
}

class RoomMgrModule
	: public block::modules::LuaAdapterModule
{
public:
	RoomMgrModule()
		: block::modules::LuaAdapterModule("RoomMgrModule")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "room_mgr.lua";
	}

private:
};


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	new block::Application();

	block::AppConfig cfg;
	cfg.tick_count = 50;
	cfg.name = "dynamic";
	APP.initCfg(cfg);

	APP.createModule(new block::network::TcpAcceptorModule("StaticTcpAcceptor"));
	APP.createModule(new RoomMgrModule);

	APP.run();

	return 0;
}
