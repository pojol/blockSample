
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
#include <lua_proxy/lua_proxy.h>
#include <distributed/node.h>
#include <timer/timer.h>

#include <log/log.h>


class GameModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	GameModule()
		: Module("GameModule")
	{}

	void before_init() override
	{
		log_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		acceptor_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("AcceptorModule"))->pop_moduleid();
		node_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("NodeModule"))->pop_moduleid();
		cfg_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("CfgModule"))->pop_moduleid();
	}

	void init() override
	{
		node_id_ = 7001;
		acceptor_ip_ = "127.0.0.1";
		acceptor_port_ = 7001;

		auto _connect_len = 1;		
		auto _cip = "127.0.0.1";
		auto _cport = 8001;

		auto _root_ip = "127.0.0.1";
		auto _root_port = 10001;

		auto _args = gsf::ArgsPool::get_ref().get();
		_args->push(node_id_);
		_args->push(get_module_id());
		_args->push("game");

		_args->push(acceptor_ip_);
		_args->push(acceptor_port_);

		_args->push(_root_ip);
		_args->push(_root_port);

		auto _module_len = 1;
		_args->push(_module_len);

		auto _module = "GameModule";
		_args->push(_module);

		auto _module_id = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args(_module))->pop_moduleid();
		assert(_module_id != gsf::ModuleNil);
		_args->push(_module_id);

		auto _module_characteristic = 0;
		_args->push(_module_characteristic);

		dispatch(node_m_, eid::distributed::node_create, _args);

		listen(this, eid::distributed::node_create_succ, [&](const gsf::ArgsPtr &args) {
			std::cout << "create_node_succ" << std::endl;
			dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), acceptor_ip_, acceptor_port_));

			return nullptr;
		});

		listen(this, eid::network::dis_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "dis connect fd = " << args->pop_fd() << std::endl;
			rpc("CoodinatorModule", eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GameModule", 0, -1));

			return nullptr;
		});

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "new connect fd = " << args->pop_fd() << std::endl;
			rpc("CoodinatorModule", eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GameModule", 0, 1));

			return nullptr;
		});
	}

private:
	int32_t node_id_ = 0;

	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID acceptor_m_ = gsf::ModuleNil;
	gsf::ModuleID node_m_ = gsf::ModuleNil;
	gsf::ModuleID cfg_m_ = gsf::ModuleNil;

	std::string acceptor_ip_ = "";
	int32_t acceptor_port_ = 0;
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
	app.regist_module(new gsf::modules::LuaProxyModule);
	app.regist_module(new gsf::modules::NodeModule);
	app.regist_module(new gsf::modules::TimerModule);

	app.regist_module(new GameModule);

	app.run();

	return 0;
}