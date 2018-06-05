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
#include <dbProxy/mysqlConnect.h>
#include <dbProxy/redisConnect.h>

#include <utils/timer.hpp>
#include <utils/logger.hpp>

#include <iostream>
#include <thread>


class TestLuaModule
	: public block::modules::LuaAdapterModule
{
public:
	TestLuaModule()
		: block::modules::LuaAdapterModule("TestLuaModule")
	{
		dir_ = "C:/github/blockSample/script";
		name_ = "hotfix.lua";
	}

private:
};

class GetCharModule
	: public block::Module
{
public:
	GetCharModule()
		: block::Module("GetCharModule")
	{
	}

	void getChar()
	{
		while (true)
		{
			std::string line_ = "";

			getline(std::cin, line_, '\n');
			if (line_ == "reload") {
				dispatch(luaM_, eid::lua::reload, nullptr);
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
	block::ModuleID luaM_ = block::ModuleNil;
};

int main()
{
	block::Application app;
	block::AppConfig cfg;
	
	APP.initCfg(cfg);

	APP.createModule(new TestLuaModule);
	APP.createModule(new GetCharModule);

	APP.run();

	return 0;
}
