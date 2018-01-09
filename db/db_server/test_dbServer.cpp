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

#include <lua_proxy/lua_proxy.h>
#include <mysql_proxy/mysql_proxy.h>
#include <distributed/node.h>
#include <timer/timer.h>

#include <log/log.h>
#include <iostream>

enum dbproxy_event
{
	db_update = 3000,
	db_sql,
	db_sqlcb,
};

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

class DBNodeProxyModule
	: public gsf::Module
	, public gsf::IEvent
{
public:
	DBNodeProxyModule()
		: Module("DBNodeProxyModule")
	{}

	virtual ~DBNodeProxyModule() {}

	void before_init() override
	{
		lua_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LuaProxyModule"))->pop_moduleid();
		assert(lua_m_ != gsf::ModuleNil);

		auto path_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("PathModule"))->pop_moduleid();
		assert(path_m_ != gsf::ModuleNil);

		lua_path_ = dispatch(path_m_, eid::sample::get_proc_path, nullptr)->pop_string();

	}

	void init() override
	{
		dispatch(lua_m_, eid::lua_proxy::create, gsf::make_args(get_module_id(), lua_path_, "dbproxy/dbServerNode.lua"));
	}

	void shut() override
	{

	}

private:
	gsf::ModuleID lua_m_ = gsf::ModuleNil;
	std::string lua_path_ = "";
};

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

		acceptor_m_ = dispatch(eid::base::app_id, eid::base::get_module, gsf::make_args("AcceptorModule"))->pop_moduleid();
		assert(acceptor_m_ != gsf::ModuleNil);
	}

	void init() override
	{
		listen(this, 10001, [&](const gsf::ArgsPtr &args) {
			
			acceptor_ip_ = args->pop_string();
			acceptor_port_ = args->pop_i32();
			node_id_ = args->pop_i32();

			dispatch(log_m_, eid::log::print, gsf::log_info("DBProxyServerModule", "node init succ!"));
			auto _res = dispatch(acceptor_m_, eid::network::make_acceptor, gsf::make_args(get_module_id(), acceptor_ip_, acceptor_port_))->pop_bool();
			if (_res) {

				bool _res = dispatch(db_p_, eid::db_proxy::mysql_connect, gsf::make_args("192.168.50.130", "root", "root", "Test", 3306))->pop_bool();
				if (_res) {
					dispatch(log_m_, eid::log::print, gsf::log_info("DBProxyServerModule", "db proxy init succ!"));
				}
				else {
					dispatch(log_m_, eid::log::print, gsf::log_error("DBProxyServerModule", "mysql connector init fail!"));
				}
			}

			return nullptr;
		});
		
		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
		
			std::cout << "new connect!" << std::endl;

			return nullptr;
		});

		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {
			
			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();

			if (_msgid == eid::db_proxy::mysql_query) {
				std::string _sql = args->pop_string();
				std::cout << "execute sql " << _sql << std::endl;

				//dispatch(db_p_, eid::db_proxy::mysql_query, gsf::make_args(_sql));
			}

			return nullptr;
		});
	}

	void shut() override
	{

	}

private:

	gsf::ModuleID log_m_ = gsf::ModuleNil;
	gsf::ModuleID db_p_ = gsf::ModuleNil;
	gsf::ModuleID acceptor_m_ = gsf::ModuleNil;
	gsf::ModuleID node_m_ = gsf::ModuleNil;

	int32_t node_id_ = 0;

	std::string acceptor_ip_ = "";
	int32_t acceptor_port_ = 0;
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
	app.regist_module(new gsf::modules::NodeModule);
	app.regist_module(new gsf::network::AcceptorModule);
	app.regist_module(new gsf::modules::MysqlProxyModule);
	app.regist_module(new gsf::modules::LuaProxyModule);
	app.regist_module(new gsf::modules::TimerModule);

	app.regist_module(new PathModule);
	app.regist_module(new DBNodeProxyModule);
	app.regist_module(new DBProxyServerModule);

	app.run();

	return 0;
}
