
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

#include <distributed/node.h>
#include <distributed/coordinate.h>

#include <log/log.h>

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
		acceptor_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("AcceptorModule"))->pop_moduleid();
		node_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("NodeModule"))->pop_moduleid();
		coodinator_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("CoodinatorModule"))->pop_moduleid();
	}

	void init() override
	{
		dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), "127.0.0.1", 10001));

		listen(this, eid::network::dis_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "dis connect ! fd=" << args->pop_fd() << std::endl;

			return nullptr;
		});

		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {
			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();

			if (_msgid == eid::distributed::coordinat_regist) {
				auto _args = gsf::ArgsPool::get_ref().get();
				auto _len = sizeof(gsf::SessionID) + 1 + sizeof(gsf::MsgID) + 1;
				_args->push_block(args->pop_block(_len, args->get_pos()).c_str(), args->get_pos() - _len);

				//std::cout << _args->toString() << std::endl;

				_args->pop_string(); //POP module name
				if (dispatch(coodinator_m_, eid::distributed::coordinat_regist, _args)->pop_bool()) {
					dispatch(acceptor_m_, eid::network::send, gsf::make_args(_fd, eid::distributed::coordinat_regist));
				}
			}

			if (_msgid == eid::distributed::coordinat_adjust_weight) {

				auto _args = gsf::ArgsPool::get_ref().get();
				auto _len = sizeof(gsf::SessionID) + 1 + sizeof(gsf::MsgID) + 1;
				_args->push_block(args->pop_block(_len, args->get_pos()).c_str(), args->get_pos() - _len);

				_args->pop_string(); //POP module name

				dispatch(coodinator_m_, eid::distributed::coordinat_adjust_weight, _args);
			}

			if (_msgid == eid::distributed::login_select_gate) {
				auto _module_name = args->pop_string();
				auto _module_characteristic = args->pop_i32();
				auto _client_fd = args->pop_fd();
				
				auto _port = dispatch(coodinator_m_, eid::distributed::coordinat_get, gsf::make_args(_module_name, _module_characteristic))->pop_i32();
				dispatch(acceptor_m_, eid::network::send
						, gsf::make_args(_fd, eid::distributed::login_select_gate_cb, _client_fd, _port));
			}

			return nullptr;
		});
	}

private:
	gsf::ModuleID node_m_ = gsf::ModuleNil;
	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID acceptor_m_ = gsf::ModuleNil;
	gsf::ModuleID coodinator_m_ = gsf::ModuleNil;
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
	app.init_cfg(cfg);

	app.regist_module(new gsf::modules::LogModule);
	app.regist_module(new gsf::network::AcceptorModule);
	app.regist_module(new gsf::modules::CoodinatorModule);

	app.regist_module(new RootModule);
	

	app.run();

	return 0;
}
