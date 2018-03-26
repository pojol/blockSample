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
#include <dbProxy/mysqlProxy.h>
#include <log/log.h>
#include <timer/timer.h>
#include <iostream>
#include <thread>


class TestLuaModule
	: public gsf::modules::LuaAdapterModule
{
public:
	TestLuaModule()
		: gsf::modules::LuaAdapterModule("TestLuaModule")
	{
		dir_ = "C:/github/gsf_sample/script";
		name_ = "hotfix.lua";
	}

private:
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
		luaM_ = APP.getModule("TestLuaModule");

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
	app.initCfg(cfg);

	APP.createModule(new gsf::modules::LogModule);
	APP.createModule(new gsf::modules::TimerModule);
	APP.createModule(new gsf::modules::MysqlProxyModule);

	APP.createModule(new TestLuaModule);
	APP.createModule(new GetCharModule);

	app.run();

	return 0;
}
