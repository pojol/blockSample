
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

				auto _callbackid = args->pop_i64();

				auto _args = gsf::ArgsPool::get_ref().get();
				auto _len = sizeof(gsf::SessionID) + 1 + sizeof(gsf::MsgID) + 1 + sizeof(int64_t) + 1;
				_args->push_block(args->pop_block(_len, args->get_pos()).c_str(), args->get_pos() - _len);

				if (dispatch(coodinator_m_, eid::distributed::coordinat_regist, _args)->pop_bool()) {
					dispatch(acceptor_m_, eid::network::send, gsf::make_args(_fd, eid::distributed::coordinat_regist, _callbackid, true, -1));
				}
			}

			if (_msgid == eid::distributed::coordinat_adjust_weight) {

				auto _args = gsf::ArgsPool::get_ref().get();
				auto _len = sizeof(gsf::SessionID) + 1 + sizeof(gsf::MsgID) + 1;
				_args->push_block(args->pop_block(_len, args->get_pos()).c_str(), args->get_pos() - _len);

				dispatch(coodinator_m_, eid::distributed::coordinat_adjust_weight, _args);
			}

			if (_msgid == eid::distributed::coordinat_select) {
				auto _callbackid = args->pop_i64();
				auto _moduleName = args->pop_string();
				auto _moduleFeature = args->pop_i32();

				std::string _block = "";
				if (args->get_tag() != 0) {
					// string = tag + typeLen + str
					auto _len = sizeof(gsf::SessionID) + 1 + sizeof(gsf::MsgID) + 1 + _moduleName.size() + 3 + sizeof(int32_t) + 1 + sizeof(int64_t) + 1;
					_block = args->pop_block(_len, args->get_pos());
				}

				auto _nodeinfo = dispatch(coodinator_m_, eid::distributed::coordinat_select, gsf::make_args(_moduleName, _moduleFeature));
				if (nullptr != _nodeinfo) {
					auto _args = gsf::ArgsPool::get_ref().get();
					_args->push(_fd);
					_args->push(eid::distributed::coordinat_select);
					_args->push(_callbackid);
					_args->push(true);
					_args->push(-1);
					_args->push_block(_nodeinfo->pop_block(0, _nodeinfo->get_pos()).c_str(), _nodeinfo->get_pos());

					if (_block != "") {
						_args->push_block(_block.c_str(), _block.size());
					}

					dispatch(acceptor_m_, eid::network::send, _args);
				}
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
	//cfg.is_watch_pref = true;
	app.init_cfg(cfg);

	app.regist_module(new gsf::modules::LogModule);
	app.regist_module(new gsf::network::AcceptorModule);
	app.regist_module(new gsf::modules::CoodinatorModule);

	app.regist_module(new RootModule);

	app.run();

	return 0;
}
