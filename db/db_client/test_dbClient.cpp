#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>

#include <fmt/format.h>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <core/application.h>
#include <core/event.h>

#include <network/acceptor.h>
#include <network/connector.h>

#include <mysql_proxy/mysql_proxy.h>

#include <log/log.h>
#include <iostream>


class EntityModule
	: public gsf::IEvent
	, public gsf::Module
{
public:

	EntityModule()
		: Module("EntityModule")
	{}

	virtual ~EntityModule() {}

	void before_init()
	{

	}

	void init()
	{

	}

	void shut()
	{

	}

};


int main()
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1, 1);
	int result = WSAStartup(wVersionRequested, &wsaData);
	if (result != 0) {
		exit(1);
	}
#endif // WIN32

	gsf::Application app;
	gsf::AppConfig cfg;
	app.init_cfg(cfg);

	app.regist_module(gsf::EventModule::get_ptr());
	app.regist_module(new gsf::modules::LogModule());
	app.regist_module(new gsf::modules::MysqlProxyModule);

	app.regist_module(new EntityModule);

	app.run();

	return 0;
}
