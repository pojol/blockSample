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
#include <log/log.h>
#include <timer/timer.h>


class TestLuaModule
	: public gsf::modules::LuaAdapterModule
{
public:
	TestLuaModule()
		: gsf::modules::LuaAdapterModule("TestLuaModule")
	{
		dir_ = "C:/github/gsf_sample/script";
		name_ = "hello_world.lua";
	}

private:
};

int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	app.initCfg(cfg);

	APP.createModule(new gsf::modules::LogModule);
	APP.createModule(new gsf::modules::TimerModule);

	APP.createModule(new TestLuaModule);

	app.run();

	return 0;
}
