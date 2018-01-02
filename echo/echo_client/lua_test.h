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

class PathModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	PathModule()
		: Module("PathModule")
	{}

	virtual ~PathModule() {}

	void before_init()
	{
		char _path[512];

#ifdef WIN32
		GetModuleFileName(NULL, _path, 512);
		//ȡ���ļ�·��
		for (int i = strlen(_path); i >= 0; i--)
		{
			if (_path[i] == '\\')
			{
				_path[i] = '\0';
				break;
			}
		}
#else
		int cnt = readlink("/proc/self/exe", _path, 512);
		if (cnt < 0 || cnt >= 512) {
			std::cout << "read path err" << std::endl;
			return;
		}
		for (int i = cnt; i >= 0; --i)
		{
			if (_path[i] == '/') {
				_path[i + 1] = '\0';
				break;
			}
		}
#endif // WIN32

		path_ = _path;

		listen(this, eid::sample::get_proc_path, [=](const gsf::ArgsPtr &args) {
			return gsf::make_args(path_);
		});
	}

private:
	std::string path_ = "";
};

class TestClientLuaModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	TestClientLuaModule()
		: Module("TestClientLuaModule")
	{}

	virtual ~TestClientLuaModule() {}

	void before_init()
	{
		luaproxy_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LuaProxyModule"))->pop_moduleid();
		gsf::ModuleID _path_m = dispatch(eid::app_id, eid::get_module, gsf::make_args("PathModule"))->pop_moduleid();
		path_ = dispatch(_path_m, eid::sample::get_proc_path, nullptr)->pop_moduleid();
	}

	void init()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), path_, "client.lua"));
	}

	void shut()
	{
		dispatch(luaproxy_m_, eid::lua_proxy::destroy, gsf::make_args(get_module_id()));
	}

private:
	uint32_t luaproxy_m_ = 0;
	std::string path_ = "";
};


namespace gsf
{
	namespace network
	{
		REGISTER_CLASS(ConnectorModule)
	}
}