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

int main()
{
	block::Application app;
	block::AppConfig cfg;
	app.initCfg(cfg);

	APP.createModule(new TestLuaModule);

	app.run();

	return 0;
}
