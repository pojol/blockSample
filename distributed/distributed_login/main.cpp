
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
#include <lua_proxy/lua_proxy.h>
#include <distributed/node.h>
#include <timer/timer.h>



enum login_event
{
	to_root = 8001,
	to_client,
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

class LoginNodeProxyModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	LoginNodeProxyModule()
		: Module("LoginNodeProxyModule")
	{}

	virtual ~LoginNodeProxyModule() {}

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
		dispatch(lua_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), lua_path_, "distributed/loginNode.lua"));
	}

	void shut() override
	{

	}

private:
	gsf::ModuleID lua_m_ = gsf::ModuleNil;
	std::string lua_path_ = "";
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
		assert(log_m_ != gsf::ModuleNil);

		login_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LoginModule"))->pop_moduleid();
		assert(login_m_ != gsf::ModuleNil);

		using namespace std::placeholders;
		listen(this, login_event::to_root, std::bind(&RootModule::event_rpc, this, _1));
	}

	void init() override
	{

	}

	gsf::ArgsPtr event_rpc(const gsf::ArgsPtr &args)
	{
		auto _client_fd = args->pop_fd();
		auto _account = args->pop_string();
		auto _password = args->pop_string();
		auto _md5 = "123";

		rpc(eid::distributed::coordinat_select, get_module_id(), gsf::make_args("GameModule", 0, _client_fd), [&](const gsf::ArgsPtr &args, int32_t process, bool result) {
		
			if (result) {

				auto _nodid = args->pop_i32();
				auto _nodtype = args->pop_string();
				auto _nodweight = args->pop_ui32();
				auto _acceptorip = args->pop_string();
				auto _acceptorport = args->pop_i32();

				auto _clientfd = args->pop_fd();

				dispatch(login_m_, login_event::to_client, gsf::make_args(_clientfd, _acceptorport));
			}
		
		});

		return nullptr;
	}

private:
	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID login_m_ = gsf::ModuleNil;

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
	app.regist_module(new gsf::modules::TimerModule);
	app.regist_module(new gsf::modules::NodeModule);
	app.regist_module(new gsf::modules::LuaProxyModule);

	app.regist_module(new PathModule);
	app.regist_module(new LoginNodeProxyModule);

	app.regist_module(new RootModule);
	app.regist_module(new LoginModule);

	app.run();

	return 0;
}