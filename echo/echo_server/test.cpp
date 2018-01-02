#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>


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
#include <lua_proxy/lua_proxy.h>

#include <log/log.h>

#include <iostream>

#include <random>

#include "addressbook.pb.h"
#include <fmt/format.h>

using namespace gsf;

class EchoServerProxyModule
	: public gsf::network::AcceptorModule
{
public:
	EchoServerProxyModule()
		: AcceptorModule("EchoServerProxyModule")
	{}

private:

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

class EchoServerScriptProxy
	: public gsf::Module
	, public gsf::IEvent
{
public:
	EchoServerScriptProxy()
		: Module("EchoServerScriptProxy")
	{}

	virtual ~EchoServerScriptProxy() {}

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
		dispatch(lua_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), lua_path_, "echo_server.lua"));
	}

	void shut() override
	{

	}

private:
	gsf::ModuleID lua_m_ = gsf::ModuleNil;
	std::string lua_path_ = "";
};

class EchoServer
	: public gsf::Module
	, public gsf::IEvent
{
public:

	EchoServer()
		: Module("EchoServer")
	{
		tick_len_ = 20;	// one second
		last_tick_ = -1;
		second_pack_num_ = 0;
	}

	void before_init() override
	{
		log_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		acceptor_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("EchoServerProxyModule"))->pop_moduleid();
		script_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("EchoServerScriptProxy"))->pop_moduleid();
	}

	void init() override
	{
		//test
		listen(this, eid::network::new_connect, [=](const gsf::ArgsPtr &args) {
			dispatch(log_m_, eid::log::print, gsf::log_info("test", fmt::format("new connect fd = {}", args->pop_fd())));

			return nullptr;
		});

		listen(this, eid::network::dis_connect, [=](const gsf::ArgsPtr &args) {
			dispatch(log_m_, eid::log::print, gsf::log_info("test", fmt::format("dis connect fd = {}", args->pop_fd())));

			return nullptr;
		});

		dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), "127.0.0.1", 8001));

		//
		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {

			auto _args = gsf::ArgsPool::get_ref().get();
			
			/*
			std::cout << args->pop_fd() << std::endl;
			std::cout << args->pop_msgid() << std::endl;
			std::string _buf = args->pop_string();
			std::cout << _buf << std::endl;
			*/
			
			//_args->push_block(args->pop_block(0, args->get_pos()).c_str(), args->get_pos());
			dispatch(script_m_, 10001, args);

			return nullptr;
		});

		listen(this, 10002, [&](const gsf::ArgsPtr &args) {

			dispatch(acceptor_m_, eid::network::send, args);

			return nullptr;
		});
	}

	void execute() override
	{
		int _t = (last_tick_ + 1) % tick_len_;
		if (_t == 0) {
			std::cout << "package num : " << second_pack_num_ << std::endl;
			second_pack_num_ = 0;
		}
		last_tick_ = _t;
	}

private :
	uint32_t tick_len_;
	int32_t last_tick_;

	uint32_t second_pack_num_;

	ModuleID log_m_ = gsf::ModuleNil;
	ModuleID acceptor_m_ = gsf::ModuleNil;
	ModuleID script_m_ = gsf::ModuleNil;
};

int main()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1, 1);
	int result = WSAStartup(wVersionRequested, &wsaData);
	if (result != 0){
		exit(1);
	}
#endif // WIN32

	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.is_watch_pref = true;
	cfg.pool_args_count = 1024 * 100;
	cfg.tick_count = 20;
	cfg.name = "test_echo_server";

	app.init_cfg(cfg);

	app.regist_module(new gsf::modules::LogModule());
	app.regist_module(new EchoServerProxyModule());
	app.regist_module(new gsf::modules::LuaProxyModule);
	app.regist_module(new PathModule);

	app.regist_module(new EchoServerScriptProxy);
	app.regist_module(new EchoServer);

	app.run();

	return 0;
}
