
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

class RootConnectorModule
	: public gsf::network::ConnectorModule
{
public:
	RootConnectorModule()
		: gsf::network::ConnectorModule("RootConnectorModule")
	{}
};


enum login_event
{
	to_root = 8001,
	to_client,
};


class RootModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	RootModule()
		: Module("RootModule")
	{}

	void before_init() override
	{
		log_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		root_connector_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("RootConnectorModule"))->pop_moduleid();

		using namespace std::placeholders;
		listen(this, login_event::to_root, std::bind(&RootModule::event_rpc, this, _1));
	}

	void init() override
	{
		dispatch(root_connector_m_, eid::network::make_connector, gsf::make_args(get_module_id(), "127.0.0.1", 10001));

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "connect ! fd=" << args->pop_fd() << std::endl;

			return nullptr;
		});

		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {

			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();

			if (_msgid == eid::distributed::login_select_gate_cb) {
				auto _client_fd = args->pop_fd();
				auto _port = args->pop_i32();
				std::cout << "select gate port = " << _port << std::endl;

				auto _login_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LoginModule"))->pop_moduleid();
				dispatch(_login_m_, login_event::to_client, gsf::make_args(_client_fd, _port));
			}

			return nullptr;
		});
	}

	gsf::ArgsPtr event_rpc(const gsf::ArgsPtr &args)
	{
		auto _client_fd = args->pop_fd();
		auto _account = args->pop_string();
		auto _password = args->pop_string();
		auto _md5 = "123";

		dispatch(root_connector_m_, eid::network::send, gsf::make_args(eid::distributed::login_select_gate, "GameModule", 0, _client_fd));

		return nullptr;
	}

private:
	gsf::ModuleID log_m_ = gsf::ModuleNil;

	gsf::ModuleID root_connector_m_ = gsf::ModuleNil;
	gsf::SessionID connect2root_fd_ = gsf::SessionNil;

	bool is_regist_ = false;
};

class LoginModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	LoginModule()
		: Module("LoginModule")
	{}

	void before_init() override
	{
		log_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		acceptor_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("AcceptorModule"))->pop_moduleid();
		root_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("RootModule"))->pop_moduleid();

		using namespace std::placeholders;
		listen(this, login_event::to_client, std::bind(&LoginModule::event_send, this, _1));
	}

	void init() override
	{
		dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), "127.0.0.1", 8001));

		listen(this, eid::network::dis_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "dis connect ! fd=" << args->pop_fd() << std::endl;
			return nullptr;
		});

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "new connect ! fd = " << args->pop_fd() << std::endl;
			return nullptr;
		});

		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {
			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();

			if (_msgid == 10001) {	// client login
				std::cout << "client login!" << std::endl;

				dispatch(root_m_, login_event::to_root, gsf::make_args(_fd, "account", "password"));
			}

			return nullptr;
		});
	}

	gsf::ArgsPtr event_send(const gsf::ArgsPtr &args)
	{
		auto _client_fd = args->pop_fd();
		auto _port = args->pop_i32();

		dispatch(acceptor_m_, eid::network::send, gsf::make_args(_client_fd, 10002, std::to_string(_port)));

		return nullptr;
	}

private:
	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID acceptor_m_ = gsf::ModuleNil;
	gsf::ModuleID root_m_ = gsf::ModuleNil;
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
	app.init_cfg(cfg);

	app.regist_module(new gsf::modules::LogModule);
	app.regist_module(new gsf::network::AcceptorModule);
	app.regist_module(new RootConnectorModule);

	app.regist_module(new RootModule);
	app.regist_module(new LoginModule);

	app.run();

	return 0;
}