
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

#include <log/log.h>


class LoginConnectorModule
	: public gsf::network::ConnectorModule
{
public:
	LoginConnectorModule()
		: gsf::network::ConnectorModule("LoginConnectorModule")
	{}
};

class GateConnectorModule
	: public gsf::network::ConnectorModule
{
public:
	GateConnectorModule()
		: gsf::network::ConnectorModule("GateConnectorModule")
	{}
};


class ClientModule
	: public gsf::Module
	, public gsf::IEvent
{

public:

	ClientModule()
		: Module("ClientModule")
	{}

	
	void before_init() override
	{
		log_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		login_connector_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LoginConnectorModule"))->pop_moduleid();
		gate_connector_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("GateConnectorModule"))->pop_moduleid();
	}

	void init() override
	{
		dispatch(login_connector_m_, eid::network::make_connector, gsf::make_args(get_module_id(), "127.0.0.1", 8001));

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			auto _fd = args->pop_fd();

			dispatch(login_connector_m_, eid::network::send, gsf::make_args(10001, "login"));

			return nullptr;
		});

		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {
			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();
		
			if (_msgid == 10002) {
				auto _port = args->pop_string();
				auto iport = std::atoi(_port.c_str());
				gate_port_ = iport;

				dispatch(eid::base::app_id, eid::base::delete_module, gsf::make_args(gate_connector_m_));
				std::cout << "relogin gate! " << iport << std::endl;
			}

			return nullptr;
		});

		listen(this, eid::module_shut_succ, [&](const gsf::ArgsPtr &args) {
			auto _module_id = args->pop_moduleid();

			if (_module_id == login_connector_m_) {	// connect 2 gate!
				dispatch(gate_connector_m_, eid::network::make_connector, gsf::make_args(get_module_id(), "127.0.0.1", gate_port_));
			}

			return nullptr;
		});
	}

private:
	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID login_connector_m_ = gsf::ModuleNil;
	gsf::ModuleID gate_connector_m_ = gsf::ModuleNil;

	uint32_t gate_port_ = 0;
};


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.name = "db";
	app.init_cfg(cfg);

	app.regist_module(new gsf::modules::LogModule);
	app.regist_module(new LoginConnectorModule);
	app.regist_module(new GateConnectorModule);

	app.regist_module(new ClientModule);

	app.run();

	return 0;
}