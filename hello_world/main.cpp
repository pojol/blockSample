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

#include <core/application.h>
#include <luaAdapter/luaAdapter.h>


class TestLuaModule
	: public block::modules::LuaAdapterModule
{
public:
	TestLuaModule()
		: block::modules::LuaAdapterModule("TestLuaModule")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "hello_world.lua";
	}

private:
};

class PerformanceModule
	: public block::Module
{
public:

	PerformanceModule()
		: block::Module("PerformanceModule") 
	{
	}

	void execute() override 
	{
		auto _luaModule = APP.getModule("TestLuaModule");

		for (int i = 0; i < 1; ++i)
		{
			dispatch(_luaModule, 10001, block::makeArgs(i, std::string("tick")));
		}
	}

private:
	
};


int main()
{
	block::Application app;
	block::AppConfig cfg;
	cfg.tick_count = 50;
	
	APP.initCfg(cfg);

	APP.createModule(new TestLuaModule);
	APP.createModule(new PerformanceModule);

	APP.run();

	return 0;
}
