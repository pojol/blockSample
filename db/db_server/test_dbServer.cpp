﻿#include <signal.h>
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
#include <stack>

enum TaskState
{
	TS_Waiting,
	TS_Querying,
	TS_Complete,
};

enum TaskType
{
	TT_Nil,
	TT_Query,
	TT_Execute
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

// test
struct Task
{
	gsf::ModuleID target_ = gsf::ModuleNil;
	TaskState state_ = TaskState::TS_Waiting;
	std::string sql_ = "";
	gsf::SessionID fd_ = gsf::SessionNil;
	int progress_ = 0;
	int64_t callbackid_ = 0;

	std::string params;
	int params_len = 0;
	TaskType type_ = TaskType::TT_Nil;
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

	void addTask(gsf::ModuleID target, const Task &task)
	{
		auto _itr = task_map_.find(target);
		if (_itr != task_map_.end()) {
			_itr->second.push(task);
		}
		else {
			std::stack<Task> _stack;
			_stack.push(task);
			task_map_.insert(std::make_pair(target, _stack));
		}
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
			auto _callbackid = args->pop_i64();

			if (_msgid == eid::distributed::mysql_query) {
				
				auto _target = args->pop_moduleid();
				std::string _sql = args->pop_string();
				std::cout << "query sql " << _sql << std::endl;

				auto _t = Task();
				_t.fd_ = _fd;
				_t.sql_ = _sql;
				_t.target_ = _target;
				_t.callbackid_ = _callbackid;
				_t.type_ = TaskType::TT_Query;

				addTask(_target, _t);
			}
			else if (_msgid == eid::distributed::mysql_execute) {
				auto _target = args->pop_moduleid();
				std::string _sql = args->pop_string();
				std::cout << "execute sql " << _sql << std::endl;

				auto _len = sizeof(gsf::SessionID) + 1 + sizeof(int32_t) + 1 + sizeof(int64_t) + 1 + sizeof(gsf::ModuleID) + 1 + _sql.size() + 3;

				auto _t = Task();
				_t.sql_ = _sql;
				_t.type_ = TaskType::TT_Execute;
				_t.params = args->pop_block(_len, args->get_size());
				_t.params_len = args->get_size() - _len;
		
				addTask(_target, _t);
			}

			return nullptr;
		});

		listen(this, eid::mysql_callback, [&](const gsf::ArgsPtr &args) {
		
			auto _remote = args->pop_moduleid();
			auto _itr = task_map_.find(_remote);
			assert(_itr != task_map_.end());

			auto _state = args->pop_bool();
			auto _progress = args->pop_i32();

			auto _fd = _itr->second.top().fd_;
			auto _callbackid = _itr->second.top().callbackid_;

			if (_state == false) {
				auto _err = args->pop_string();
				if (_callbackid != 0) {
					dispatch(acceptor_m_, eid::network::send, gsf::make_args(_fd, eid::distributed::mysql_query, _callbackid, _state, _progress, _err));
				}
				else {
					dispatch(log_m_, eid::log::print, gsf::log_warring("DBServer", _err));
				}
				_itr->second.pop();
			}
			else {
				_itr->second.top().progress_ = _progress;

				if (_progress != -1) {

					if (_callbackid != 0) {
						auto _args = gsf::ArgsPool::get_ref().get();
						_args->push(_fd);
						_args->push(int(eid::distributed::mysql_query));
						_args->push(_callbackid);
						_args->push(_state);
						_args->push(_progress);
						auto _len = sizeof(bool) + 1 + sizeof(int32_t) + 1 + sizeof(int32_t) + 1;
						_args->push_block(args->pop_block(_len, args->get_size()).c_str(), args->get_size() - _len);

						dispatch(acceptor_m_, eid::network::send, _args);
					}

				}
				else { // complete
					if (_callbackid != 0) {
						dispatch(acceptor_m_, eid::network::send, gsf::make_args(_fd, eid::distributed::mysql_query, _callbackid, _state, _progress));
					}
					_itr->second.pop();
				}
			}

			return nullptr;
		});
	}

	void execute() override
	{
		for (auto it : task_map_)
		{
			if (!it.second.empty()) {

				auto _tsk = it.second.top();

				if (_tsk.state_ == TaskState::TS_Waiting) {

					it.second.top().state_ = TaskState::TS_Querying;

					if (_tsk.type_ == TaskType::TT_Query) {
						dispatch(db_p_, eid::distributed::mysql_query, gsf::make_args(get_module_id(), _tsk.target_, _tsk.sql_));
					}
					else {

						auto _args = gsf::ArgsPool::get_ref().get();
						_args->push_string(_tsk.sql_);
						_args->push_block(_tsk.params.c_str(), _tsk.params_len);

						dispatch(db_p_, eid::distributed::mysql_execute, _args);

						//
						it.second.pop();
					}
				}
			}	
		}
	}

	void shut() override
	{

	}

private:
	//! 保证sql执行循序先进先出
	std::unordered_map<gsf::ModuleID, std::stack<Task >> task_map_;


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
