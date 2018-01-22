#include "cpp_test.h"
#include "lua_test.h"

void case_cpp_client(gsf::Application &app)
{
	for (int i = 0; i < 1; ++i)
	{
		app.create_module(new Client);
	}
}

void case_lua_client(gsf::Application &app)
{
	app.create_module(new gsf::modules::LuaProxyModule);
	app.create_module(new PathModule);
	app.create_module(new TestClientLuaModule);
}


int main()
{
#ifdef WIN32
	WSADATA wsa_data;
	WSAStartup(0x0201, &wsa_data);
#endif

	gsf::Application app;
	gsf::AppConfig cfg;
	cfg.name = "test_echo_client";
	cfg.pool_args_count = 1024 * 100;
	app.init_cfg(cfg);

	app.create_module(new gsf::modules::LogModule);
	app.create_module(new gsf::network::ConnectorModule);
	app.create_module(new gsf::modules::TimerModule);

	case_cpp_client(app);
	//case_lua_client(app);

	app.run();

	return 0;
}
