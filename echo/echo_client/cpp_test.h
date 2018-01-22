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
#include <core/event.h>
#include <core/dynamic_module_factory.h>

#include <network/acceptor.h>
#include <network/connector.h>
#include <timer/timer.h>

#include <log/log.h>

#include <random>
#include <lua_proxy/lua_proxy.h>

#include "addressbook.pb.h"

class Client
	: public gsf::Module
	, public gsf::IEvent
{
public:
	Client()
		: Module("Client")
	{}

	void before_init() override
	{
		connector_id_ = APP.create_dynamic_module("ConnectorModule");
		assert(connector_id_ != gsf::ModuleNil);

		timer_m_ = APP.get_module("TimerModule");
		assert(timer_m_ != gsf::ModuleNil);

		listen(this, eid::timer::timer_arrive, std::bind(&Client::timerArrive, this, std::placeholders::_1));

		listen(connector_id_, eid::base::module_init_succ, std::bind(&Client::create_connector_succ, this, std::placeholders::_1));
	}

	gsf::ArgsPtr timerArrive(const gsf::ArgsPtr &args)
	{
		auto _t = args->pop_timerid();
		if (_t == timer_id_) {

			std::string _msg = "hello";
			dispatch(connector_id_, eid::network::send, gsf::make_args(1001, _msg));

			timer_id_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 20))->pop_timerid();
		}
		else if (_t == tick_id_) {

			std::cout << package_num << std::endl;
			package_num = 0;
			tick_id_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 1000))->pop_timerid();
		}

		return nullptr;
	}

	gsf::ArgsPtr create_connector_succ(const gsf::ArgsPtr &args)
	{
		// boardcast 的消息不要把里面的内容取出来, 会影响到后续的接收者。 （这个地方以后要用机制保证下
		auto _t = gsf::ArgsPool::get_ref().get();
		_t->push_block(args->pop_block(0, args->get_size()).c_str(), args->get_size());

		auto _module_id = _t->pop_moduleid();
		if (_module_id != connector_id_) { return nullptr; }

		listen(this, eid::network::new_connect, [&](const gsf::ArgsPtr &args) {
			fd_ = args->pop_fd();
			/*
				tutorial::Person _p;
				_p.set_name("jack");
				_p.set_id(1000);
				_p.set_email("127.0.0.1");
			*/

			//timer_id_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 20))->pop_timerid();
			//tick_id_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 1000))->pop_timerid();

			std::string _msg = "hello";

			//if (_p.SerializeToString(&_msg)) {
			dispatch(connector_id_, eid::network::send, gsf::make_args(1001, _msg));
			//}

			// 分布式rpc调用接口预定义
			//dispatch(node_m_, eid::login::auth, gsf::make_args("account", "password", "verify_key"));

			return nullptr;
		});
		
		listen(this, eid::network::recv, [&](const gsf::ArgsPtr &args) {
			
			auto _fd = args->pop_fd();
			auto _msgid = args->pop_msgid();
			if (_msgid == 1002) {
				package_num++;
				
				std::string _msg = "hello";
				dispatch(connector_id_, eid::network::send, gsf::make_args(1001, _msg));
			}

			return nullptr;
		});

		dispatch(connector_id_, eid::network::make_connector
			, gsf::make_args(get_module_id(), "127.0.0.1", 8001));

		return nullptr;
	}

	void init() override
	{

	}

private:
	int package_num = 0;

	gsf::ModuleID connector_id_ = gsf::ModuleNil;
	gsf::ModuleID timer_m_ = gsf::ModuleNil;

	gsf::TimerID timer_id_ = gsf::TimerNil;
	gsf::TimerID tick_id_ = gsf::TimerNil;
	gsf::SessionID fd_ = gsf::SessionNil;
};