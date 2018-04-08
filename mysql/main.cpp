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

#include <timer/timer.h>

#include <log/log.h>
#include <luaAdapter/luaAdapter.h>

#include "DBEntity.hpp"
#include <thread>

class TestCaseLuaModule
	: public gsf::modules::LuaAdapterModule
{
public:
	TestCaseLuaModule()
		: gsf::modules::LuaAdapterModule("TestCaseLuaModule")
	{
		dir_ = "c:/github/blockSample/script";
		name_ = "test_mysql.lua";
	}

	virtual ~TestCaseLuaModule() {}

private:
	uint32_t luaproxy_m_ = 0;
};

class GetCharModule
	: public gsf::Module
{
public:
	GetCharModule()
		: gsf::Module("GetCharModule")
	{
	}

	void getChar()
	{
		while (true)
		{
			std::string line_ = "";

			getline(std::cin, line_, '\n');
			if (line_ == "reload") {
				mailboxPtr_->dispatch(luaM_, eid::lua::reload, nullptr);
			}

			Sleep(10);
		}
	}

	void init() override
	{
		luaM_ = APP.getModule("TestCaseLuaModule");

		std::thread t1(&GetCharModule::getChar, this);
		t1.detach();
	}

private:
	gsf::ModuleID luaM_ = gsf::ModuleNil;
};


int main()
{
	gsf::Application app;
	gsf::AppConfig cfg;
	//cfg.is_watch_pref = true;
	app.initCfg(cfg);

	APP.createModule(new gsf::modules::LogModule);
	APP.createModule(new gsf::modules::TimerModule);

	APP.createModule(new GetCharModule);
	APP.createModule(new TestCaseLuaModule);
	APP.createModule(new DBEntityModule);

	app.run();

	return 0;
}
