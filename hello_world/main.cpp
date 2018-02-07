#include <core/application.h>
#include <core/event.h>

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <lua_proxy/lua_proxy.h>
#include <log/log.h>
#include <timer/timer.h>


class TestLuaModuleCtl
	: public gsf::Module
	, public gsf::IEvent
{
public:
	TestLuaModuleCtl()
		: Module("TestLuaModuleCtl")
	{}

	virtual ~TestLuaModuleCtl() {}

	void before_init()
	{
		luaproxy_m_ = APP.get_module("LuaProxyModule");
		assert(luaproxy_m_ != gsf::ModuleNil);
	}

	void init()
	{
		dispatch(luaproxy_m_
			, eid::lua_proxy::create
			, gsf::make_args(get_module_id(), "hello_world.lua"));
	}

	void shut()
	{
		dispatch(luaproxy_m_
			, eid::lua_proxy::destroy
			, gsf::make_args(get_module_id()));
	}

private:
	uint32_t luaproxy_m_ = gsf::ModuleNil;
};



int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	app.init_cfg(cfg);

	APP.create_module(new gsf::modules::LogModule);
	APP.create_module(new gsf::modules::TimerModule);
	APP.create_module(new gsf::modules::LuaProxyModule);

	APP.create_module(new TestLuaModuleCtl);

	app.run();

	return 0;
}
