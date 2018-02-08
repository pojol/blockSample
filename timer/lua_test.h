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
		luaproxy_m_ = APP.get_module("LuaProxyModule");
		assert(luaproxy_m_ != gsf::ModuleNil);
	}

	void init()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), "test_timer.lua"));
	}

	void shut()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::destroy, gsf::make_args(get_module_id()));
	}

private :
	uint32_t luaproxy_m_ = 0;
};