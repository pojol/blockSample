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

#include <timer/timer.h>

#include <log/log.h>
#include <luaProxy/luaProxy.h>
#include <dbProxy/mysqlProxy.h>

class TestCaseLuaModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	TestCaseLuaModule()
		: Module("TestCaseLuaModule")
	{}

	virtual ~TestCaseLuaModule() {}

	void before_init()
	{
		luaproxy_m_ = APP.getModule("LuaProxyModule");
		assert(luaproxy_m_ != gsf::ModuleNil);
	}

	void init()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::makeArgs(getModuleID(), "test_mysql.lua"));
	}

	void shut()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::destroy, gsf::makeArgs(getModuleID()));
	}

private:
	uint32_t luaproxy_m_ = 0;
};


int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.scriptPath_ = "E:/github/gsf_sample/script";
	//cfg.is_watch_pref = true;
	app.initCfg(cfg);

	APP.createModule(new gsf::modules::LogModule);
	APP.createModule(new gsf::modules::LuaProxyModule);
	APP.createModule(new gsf::modules::TimerModule);
	APP.createModule(new gsf::modules::MysqlProxyModule);

	APP.createModule(new TestCaseLuaModule);

	app.run();

	return 0;
}
