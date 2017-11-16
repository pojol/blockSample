
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


class GateLoginModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	GateLoginModule()
		: Module("GateLoginModule")
	{}

	void before_init() override
	{
		dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("LogModule"), [&](const gsf::ArgsPtr &args) {
			log_m_ = args->pop_moduleid();
		});

		dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("AcceptorModule"), [&](const gsf::ArgsPtr &args) {
			acceptor_m_ = args->pop_moduleid();
		});

		dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("NodeModule"), [&](const gsf::ArgsPtr &args) {
			node_m_ = args->pop_moduleid();
		});

		dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("CfgModule"), [&](const gsf::ArgsPtr &args) {
			cfg_m_ = args->pop_moduleid();
		});
	}

	void init() override
	{
		dispatch(cfg_m_, eid::sample::get_cfg, nullptr, [&](const gsf::ArgsPtr &args) {

			node_id_ = args->pop_i32();

			acceptor_ip_ = args->pop_string();
			acceptor_port_ = args->pop_i32();

			auto _connect_len = args->pop_i32();
			for (int i = 0; i < _connect_len; ++i)
			{
				auto _cip = args->pop_string();
				auto _cport = args->pop_i32();
			}

			auto _root_ip = args->pop_string();
			auto _root_port = args->pop_i32();

			auto _args = gsf::ArgsPool::get_ref().get();
			_args->push(node_id_);
			_args->push(get_module_id());
			_args->push("gate");

			_args->push(acceptor_ip_);
			_args->push(acceptor_port_);

			_args->push(_root_ip);
			_args->push(_root_port);

			auto _module_len = args->pop_i32();
			_args->push(_module_len);

			for (int i = 0; i < _module_len; ++i)
			{
				auto _module = args->pop_string();
				_args->push(_module);

				dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args(_module), [&](const gsf::ArgsPtr &args) {
					auto _module_id = args->pop_moduleid();
					assert(_module_id != gsf::ModuleNil);
					_args->push(_module_id);
				});

				auto _module_characteristic = args->pop_i32();
				_args->push(_module_characteristic);
			}

			dispatch(node_m_, eid::distributed::node_create, _args);
		});

		listen(this, eid::distributed::node_create_succ, [&](const gsf::ArgsPtr &args, gsf::CallbackFunc callback) {
			std::cout << "create_node_succ" << std::endl;

			dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), acceptor_ip_, acceptor_port_));
		});

		listen(this, eid::network::dis_connect, [&](const gsf::ArgsPtr &args, gsf::CallbackFunc callback) {

			std::cout << "dis connect fd = " << args->pop_fd() << std::endl;

			rpc("CoodinatorModule", eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GateLoginModule", 0, -1));
		});

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args, gsf::CallbackFunc callback) {

			std::cout << "new connect fd = " << args->pop_fd() << std::endl;

			rpc("CoodinatorModule", eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GateLoginModule",0 , 1));
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

class PathModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	PathModule()
		: Module("PathModule")
	{}

	virtual ~PathModule() {}

	void before_init()
	{
		char _path[512];

#ifdef WIN32
		GetModuleFileName(NULL, _path, 512);
		//取出文件路径
		for (int i = strlen(_path); i >= 0; i--)
		{
			if (_path[i] == '\\')
			{
				_path[i] = '\0';
				break;
			}
		}
#else
		int cnt = readlink("/proc/self/exe", _path, 512);
		if (cnt < 0 || cnt >= 512) {
			std::cout << "read path err" << std::endl;
			return;
		}
		for (int i = cnt; i >= 0; --i)
		{
			if (_path[i] == '/') {
				_path[i + 1] = '\0';
				break;
			}
		}
#endif // WIN32

		path_ = _path;

		listen(this, eid::sample::get_proc_path, [=](const gsf::ArgsPtr &args, gsf::CallbackFunc callback) {
			callback(gsf::make_args(path_));
		});
	}

private:
	std::string path_ = "";
};

class CfgModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	CfgModule()
		: Module("CfgModule")
	{}

	void before_init() override
	{
		dispatch(eid::app_id, eid::get_module, gsf::make_args("LuaProxyModule"), [&](const gsf::ArgsPtr &args) {
			luaproxy_m_ = args->pop_moduleid();
		});

		dispatch(eid::app_id, eid::get_module, gsf::make_args("PathModule"), [&](const gsf::ArgsPtr &args) {
			path_m_ = args->pop_moduleid();
		});
	}

	void init() override
	{
		std::string _path = "";
		dispatch(path_m_, eid::sample::get_proc_path, nullptr, [&](const gsf::ArgsPtr &args) {
			_path = args->pop_string();
		});

		dispatch(luaproxy_m_, eid::lua_proxy::create
			, gsf::make_args(get_module_id(), _path, "cfg.lua"));
	}

private:
	gsf::ModuleID luaproxy_m_ = gsf::ModuleNil;
	gsf::ModuleID path_m_ = gsf::ModuleNil;
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

	app.regist_module(new PathModule);
	app.regist_module(new CfgModule);

	app.regist_module(new GateLoginModule);

	app.run();

	return 0;
}