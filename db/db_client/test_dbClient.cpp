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

#include <distributed/node.h>
#include <timer/timer.h>

#include <log/log.h>
#include <iostream>

#include <fmt/format.h>

#include "entity.hpp"
#include "clientNode.hpp"

class DBClientModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	DBClientModule()
		: Module("DBClientModule")
	{}

	virtual ~DBClientModule() {}

	void before_init() override
	{
		entity_mgr_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("EntityMgrModule"))->pop_moduleid();
		assert(entity_mgr_m_ != gsf::ModuleNil);
	}

	void init() override
	{
		listen(this, 10001, [&](const gsf::ArgsPtr &args) {
		
			std::cout << "init succ!" << std::endl;

			dispatch(entity_mgr_m_, 10002, nullptr);

			return nullptr;
		});


	}

	void shut() override
	{

	}

private:
	gsf::ModuleID db_connector_m_ = gsf::ModuleNil;
	gsf::ModuleID entity_mgr_m_ = gsf::ModuleNil;
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

	app.regist_module(gsf::EventModule::get_ptr());
	app.regist_module(new gsf::modules::LogModule());
	app.regist_module(new gsf::modules::NodeModule);
	app.regist_module(new gsf::modules::LuaProxyModule);
	app.regist_module(new gsf::modules::TimerModule);

	app.regist_module(new DBClientModule);
	app.regist_module(new PathModule);
	app.regist_module(new DBNodeProxyModule);
	app.regist_module(new EntityMgrModule);

	app.run();

	return 0;
}
