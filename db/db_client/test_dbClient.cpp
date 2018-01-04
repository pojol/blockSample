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

enum dbproxy_event
{
	db_update = 3000,
	db_sql,
	db_sqlcb,
};

class Client2DBConnector
	: public gsf::network::ConnectorModule
{
public:
	Client2DBConnector()
		: gsf::network::ConnectorModule("Client2DBConnector")
	{}
};

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

		connector_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("Client2DBConnector"))->pop_moduleid();
		assert(connector_m_ != gsf::ModuleNil);
	}

	void init() override
	{
		listen(this, 10001, [&](const gsf::ArgsPtr &args) {
		
			std::cout << "init succ!" << std::endl;

			rpc(eid::distributed::coordinat_select, gsf::make_args("DBProxyServerModule", 0), [&](const gsf::ArgsPtr &args, bool result) {

				if (result) {
					auto _nodid = args->pop_i32();
					auto _nodtype = args->pop_string();
					auto _nodweight = args->pop_ui32();
					auto _acceptorip = args->pop_string();
					auto _acceptorport = args->pop_i32();

					dispatch(connector_m_, eid::network::make_connector, gsf::make_args(get_module_id(), _acceptorip, _acceptorport));
				}

			});

			return nullptr;
		});

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
		
			db_fd_ = args->pop_fd();

			// init succ
			dispatch(entity_mgr_m_, 10002, nullptr);

			return nullptr;
		});

		listen(this, 10003, [&](const gsf::ArgsPtr &args) {
		
			dispatch(connector_m_, eid::network::send, gsf::make_args(dbproxy_event::db_sql, args->pop_string()));

			return nullptr;
		});

		listen(this, 10004, [&](const gsf::ArgsPtr &args) {
		
			auto _args = gsf::ArgsPool::get_ref().get();
			_args->push(dbproxy_event::db_sql);
			_args->push_block(args->pop_block(0, args->get_pos()).c_str(), args->get_pos());

			dispatch(connector_m_, eid::network::send, _args);

			return nullptr;
		});
	}

	void shut() override
	{

	}

private:
	gsf::ModuleID entity_mgr_m_ = gsf::ModuleNil;
	gsf::ModuleID connector_m_ = gsf::ModuleNil;

	gsf::SessionID db_fd_ = gsf::SessionNil;
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

	app.regist_module(new Client2DBConnector);

	app.regist_module(new DBClientModule);
	app.regist_module(new PathModule);
	app.regist_module(new DBNodeProxyModule);
	app.regist_module(new EntityMgrModule);

	app.run();

	return 0;
}
