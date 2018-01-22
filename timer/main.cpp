#include "cpp_test.h"
#include "lua_test.h"


void case_cpp_timer()
{
	APP.create_module(new TestCaseModule);
}


void case_lua_timer()
{
	APP.create_module(new PathModule);
	APP.create_module(new PathModule);
	APP.create_module(new gsf::modules::LuaProxyModule);
	APP.create_module(new TestCaseLuaModule);
}


int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	//cfg.is_watch_pref = true;
	app.init_cfg(cfg);

	APP.create_module(new gsf::modules::LogModule);
	APP.create_module(new gsf::modules::TimerModule);

	//case_cpp_timer();
	case_lua_timer();

	app.run();

	return 0;
}
