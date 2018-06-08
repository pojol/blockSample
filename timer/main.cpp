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
#include <core/dynamic_module_factory.h>

#include <luaAdapter/luaAdapter.h>


class TestCaseLuaModule
	: public block::modules::LuaAdapterModule
{
public:
	TestCaseLuaModule()
		: block::modules::LuaAdapterModule("TestCaseLuaModule")
	{}

	virtual ~TestCaseLuaModule() {}

	void before_init()
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "test_timer.lua";
	}
};


int main()
{
	new block::Application;

	block::AppConfig cfg;
	//cfg.is_watch_pref = true;
	APP.initCfg(cfg);

	APP.createModule(new TestCaseLuaModule);

	APP.run();

	return 0;
}
