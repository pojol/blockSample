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
#include <stack>

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
		lua_m_ = APP.get_module("LuaProxyModule");
		assert(lua_m_ != gsf::ModuleNil);
		
		auto path_m_ = APP.get_module("PathModule");
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
	gsf::SessionID fd_ = gsf::SessionNil;
	int progress_ = 0;
	int64_t callbackid_ = 0;
	gsf::EventID eid_ = gsf::EventNil;

	std::string params = "";
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
		log_m_ = APP.get_module("LogModule");
		assert(log_m_ != gsf::ModuleNil);

		db_p_ = APP.get_module("MysqlProxyModule");
		assert(db_p_ != gsf::ModuleNil);

		acceptor_m_ = APP.get_module("AcceptorModule");
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

			std::cout << "recv : " << _msgid << std::endl;
			std::cout << args->to_string() << std::endl;

			int64_t _callbackid = 0;
			int32_t _target = 0;
			int32_t _len = 0;

			_callbackid = args->pop_i64();
			if (eid::distributed::mysql_query == _msgid) {
				_target = args->pop_moduleid();
				_len = sizeof(gsf::SessionID) + 1 + sizeof(int32_t) + 1 + sizeof(int64_t) + 1 + sizeof(gsf::ModuleID) + 1;
			}
			else if (eid::distributed::mysql_update == _msgid)
			{
				_len = sizeof(gsf::SessionID) + 1 + sizeof(int32_t) + 1 + sizeof(int64_t) + 1;
			}

			auto _t = Task();
			_t.fd_ = _fd;
			_t.params = args->pop_block(_len, args->get_size());
			_t.target_ = _target;
			_t.callbackid_ = _callbackid;
			_t.eid_ = _msgid;

			addTask(_target, _t);

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
		for (auto &it : task_map_)
		{
			if (!it.second.empty()) {

				auto _tsk = it.second.top();

				auto _args = gsf::ArgsPool::get_ref().get();
				if (_tsk.eid_ == eid::distributed::mysql_query) {
					_args->push(get_module_id());
					_args->push(_tsk.target_);
				}

				_args->push_block(_tsk.params.c_str(), _tsk.params.size());
				std::cout << _args->to_string() << std::endl;
				dispatch(db_p_, _tsk.eid_, _args);

				if (_tsk.callbackid_ == 0) {
					it.second.pop();
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

	app.create_module(gsf::EventModule::get_ptr());
	app.create_module(new gsf::modules::LogModule());
	app.create_module(new gsf::modules::NodeModule);
	app.create_module(new gsf::network::AcceptorModule);
	app.create_module(new gsf::modules::MysqlProxyModule);
	app.create_module(new gsf::modules::LuaProxyModule);
	app.create_module(new gsf::modules::TimerModule);

	app.create_module(new PathModule);
	app.create_module(new DBNodeProxyModule);
	app.create_module(new DBProxyServerModule);

	app.run();

	return 0;
}
