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

#include <luaProxy/luaProxy.h>
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
		luaproxy_m_ = APP.getModule("LuaProxyModule");
		assert(luaproxy_m_ != gsf::ModuleNil);
	}

	void init()
	{
		dispatch(luaproxy_m_
			, eid::lua_proxy::create
			, gsf::makeArgs(getModuleID(), "hello_world.lua"));
	}

	void shut()
	{
		dispatch(luaproxy_m_
			, eid::lua_proxy::destroy
			, gsf::makeArgs(getModuleID()));
	}

private:
	uint32_t luaproxy_m_ = gsf::ModuleNil;
};


int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.scriptPath_ = "C:/github/gsf_sample/script";
	app.initCfg(cfg);

	APP.createModule(new gsf::modules::LogModule);
	APP.createModule(new gsf::modules::TimerModule);
	APP.createModule(new gsf::modules::LuaProxyModule);

	APP.createModule(new TestLuaModuleCtl);

	app.run();

	return 0;
}
