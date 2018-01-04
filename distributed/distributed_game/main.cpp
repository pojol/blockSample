
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

		listen(this, eid::sample::get_proc_path, [=](const gsf::ArgsPtr &args) {
			return gsf::make_args(path_);
		});
	}

private:
	std::string path_ = "";
};

class GameNodeProxyModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	GameNodeProxyModule()
		: Module("GameNodeProxyModule") 
	{}

	virtual ~GameNodeProxyModule() {}

	void before_init() override
	{
		lua_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LuaProxyModule"))->pop_moduleid();
		assert(lua_m_ != gsf::ModuleNil);

		auto path_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("PathModule"))->pop_moduleid();
		assert(path_m_ != gsf::ModuleNil);

		lua_path_ = dispatch(path_m_, eid::sample::get_proc_path, nullptr)->pop_string();
	}

	void init() override
	{
		dispatch(lua_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), lua_path_, "game/gameNode.lua"));
	}

	void shut() override
	{

	}

private:
	gsf::ModuleID lua_m_ = gsf::ModuleNil;
	std::string lua_path_ = "";
};

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
		listen(this, 10001, [&](const gsf::ArgsPtr &args) {
			
			acceptor_ip_ = args->pop_string();
			acceptor_port_ = args->pop_i32();
			node_id_ = args->pop_i32();

			dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), acceptor_ip_, acceptor_port_));

			return nullptr;
		});

		listen(this, eid::network::dis_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "dis connect fd = " << args->pop_fd() << std::endl;
			rpc(eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GameModule", 0, -1));

			return nullptr;
		});

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			std::cout << "new connect fd = " << args->pop_fd() << std::endl;
			rpc(eid::distributed::coordinat_adjust_weight, gsf::make_args(node_id_, "GameModule", 0, 1));

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

	app.regist_module(new PathModule);

	app.regist_module(new GameModule);
	app.regist_module(new GameNodeProxyModule);

	app.run();

	return 0;
}