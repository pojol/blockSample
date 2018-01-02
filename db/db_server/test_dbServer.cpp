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


class DBProxyServerModule
	: public gsf::IEvent
	, public gsf::Module
{
public:

	DBProxyServerModule()
		: Module("DBProxyServerModule")
	{}

	virtual ~DBProxyServerModule() {}

	void before_init() override
	{
		log_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LogModule"))->pop_moduleid();
		assert(log_m_ != gsf::ModuleNil);

		db_p_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("MysqlProxyModule"))->pop_moduleid();
		assert(db_p_ != gsf::ModuleNil);
	}

	void init() override
	{

	}

	void shut() override
	{

	}

private:

	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID db_p_ = gsf::ModuleNil;
};

void init()
{
	auto mysqlInit = mysql_init(nullptr);
	if (nullptr == mysqlInit) {
		std::cout << "err" << std::endl;
	}

	//mysqlPtr = mysql_real_connect(mysqlInit, "192.168.50.130", "root", "root", "Logs233", 3306, nullptr, 0);
	//if (nullptr == mysqlPtr) {
	//	mysql_close(mysqlPtr);
	//}

	/*
	std::string qstr = fmt::format("create table if not exists Test1{}{}{}{}{}{}", "( "
		, "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		, "name VARCHAR(32) NOT NULL,"
		, "time INT NOT NULL" 
		, ")"
		, " ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;");
	query(qstr);
	*/

	//SqlStmtPtr stmt;
	//perpare("insert into Test1 values (?,?,?)", stmt);
	//ExecuteStmt(stmt, gsf::make_args(1, "hello", 200));

	//SqlStmtPtr stmt;
	//perpare("insert into Consume20171207 values (?,?,?,?,?,?,?,?)", stmt);
	//ExecuteStmt(stmt, gsf::make_args(111, 10, 100, 1000, 10000, 100000, 2, "hello"));

	
	std::cout << "" << std::endl;
}

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

	app.regist_module(new DBProxyServerModule);

	app.run();

	return 0;
}
