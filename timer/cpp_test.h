#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>

#include <iostream>
#include <random>
#include <tuple>

#include <core/application.h>
#include <core/event.h>

#include <timer/timer.h>
#include <log/log.h>

#include <fmt/format.h>

struct TestCaseModule
	: public gsf::Module
	, public gsf::IEvent
{
	TestCaseModule()
		: Module("TestCaseModule")
	{}

	virtual ~TestCaseModule() {}

	void before_init() override
	{
		timer_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("TimerModule"))->pop_moduleid();
		log_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("LogModule"))->pop_moduleid();
	}

	gsf::ArgsPtr timeArrive(const gsf::ArgsPtr &args)
	{
		auto _timer_id = args->pop_timerid();

		if (_timer_id == millisecondsTimerID_) {
			dispatch(log_m_, eid::log::print, gsf::log_info("TestCaseModule", fmt::format("time arrive id={}", _timer_id)));

			//! 
			millisecondsTimerID_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 1000))->pop_timerid();
		}

		return nullptr;
	}

	void init() override
	{
		millisecondsTimerID_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(get_module_id(), 1000))->pop_timerid();

		listen(this, eid::timer::timer_arrive, std::bind(&TestCaseModule::timeArrive, this, std::placeholders::_1));
	}

private:
	uint32_t timer_m_;
	uint32_t log_m_;

	gsf::TimerID millisecondsTimerID_;
};