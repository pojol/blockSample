﻿#pragma once

#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <memory>

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#endif // WIN32

#include <core/application.h>
#include <core/dynamic_module_factory.h>

#include <timer/timer.h>

#include <log/log.h>
#include <luaAdapter/luaAdapter.h>

#include <dbProxy/mysqlConnect.h>
#include <dbProxy/redisConnect.h>

#include "Avatar.pb.h"

//! auto gen file

class DBEntityModule
	: public gsf::Module
{
public:

	DBEntityModule()
		: gsf::Module("DBEntityModule")
	{

	}

protected:
	void before_init() override
	{
		using namespace std::placeholders;

		mailboxPtr_->listen(eid::dbProxy::connect
			, std::bind(&DBEntityModule::eInit, this, std::placeholders::_1, std::placeholders::_2));

		mailboxPtr_->listen(eid::dbProxy::execSql
			, std::bind(&DBEntityModule::eExecSql, this, std::placeholders::_1, std::placeholders::_2));

		mailboxPtr_->listen(eid::dbProxy::load
			, std::bind(&DBEntityModule::eLoad, this, std::placeholders::_1, std::placeholders::_2));

		mailboxPtr_->listen(eid::dbProxy::insert
			, std::bind(&DBEntityModule::eInsert, this, std::placeholders::_1, std::placeholders::_2));

		mailboxPtr_->listen(eid::dbProxy::update
			, std::bind(&DBEntityModule::eUpdate, this, std::placeholders::_1, std::placeholders::_2));
	
		mysqlPtr_ = std::make_shared<gsf::modules::MysqlConnect>();
	}

	void init() override
	{
		mailboxPtr_->pull();
	}

	void execute() override
	{
		mailboxPtr_->pull();

		while (!queue_.empty())
		{
			auto _callback = queue_.front();

			mailboxPtr_->dispatch(_callback->target_, eid::dbProxy::callback, std::move(_callback->args_));

			delete _callback;
			_callback = nullptr;

			queue_.pop();
		}
	}

	void shut() override
	{
		mailboxPtr_->pull();
	}

private:
	
	void eInit(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		auto _host = args->pop_string();	//host
		auto _user = args->pop_string(); //user
		auto _password = args->pop_string(); //password
		auto _dbName = args->pop_string(); //database
		auto _port = args->pop_i32();	//port
		auto _useCache = args->pop_bool();

		if (!mysqlPtr_->init(_host, _port, _user, _password, _dbName)) {
			APP.ERR_LOG("MysqlProxy", "init fail!");
			return;
		}

		useCache_ = _useCache;
		if (useCache_) {
			timerM_ = APP.getModule("TimerModule");
			if (timerM_ == gsf::ModuleNil) {
				APP.ERR_LOG("DBProxy", "unRegist TimerModule!");
				return;
			}

			redisPtr_ = std::make_shared<gsf::modules::RedisConnect>();
			redisPtr_->init();


			mailboxPtr_->listen(eid::timer::timer_arrive, std::bind(&DBEntityModule::onTimer, this, std::placeholders::_1, std::placeholders::_2));

			mailboxPtr_->dispatch(timerM_, eid::timer::delay_milliseconds, gsf::makeArgs(TimerType::tt_command, execDelay_));
			mailboxPtr_->dispatch(timerM_, eid::timer::delay_milliseconds, gsf::makeArgs(TimerType::tt_rewrite, rewriteDelay_));
		}
	}

	/*!
	获取一个实例
	**/
	void eLoad(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		auto _id = args->pop_i32();

		mysqlPtr_->execSql(target, eid::dbProxy::load, "select * from Entity where id = '1';", [&](gsf::ModuleID _target, gsf::ArgsPtr args) {
			auto _callbackPtr = new CallbackInfo();
			
			_callbackPtr->args_ = std::move(args);
			_callbackPtr->target_ = _target;
			queue_.push(_callbackPtr);
		});
	}

	/*!
	创建一个实例
	**/
	void eInsert(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		auto _buf = args->exportBuf();

		//test::Avatar _avatar;
		//_avatar.ParseFromArray(_buf.c_str(), args->get_size());

		if (useCache_) {

		}
		else {
			mysqlPtr_->insert("insert into Entity values(?, ?);", _buf.c_str(), args->get_size());
		}
	}

	/*!
	更新一个实例
	**/
	void eUpdate(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		auto _id = args->pop_i32();
		auto _buf = args->pop_string();

		if (useCache_) {

		}
		else {
			mysqlPtr_->update("update Entity set dat = ? where id = " + std::to_string(_id) + ";", _buf.c_str(), _buf.length());
		}
	}

	/*!
	执行一条sql语句
	**/
	void eExecSql(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		std::string queryStr = args->pop_string();

		using namespace std::placeholders;

		mysqlPtr_->execSql(target, eid::dbProxy::execSql, queryStr, [&](gsf::ModuleID _target, gsf::ArgsPtr args) {
			auto _callbackPtr = new CallbackInfo();
			_callbackPtr->args_ = std::move(args);
			_callbackPtr->target_ = _target;
			queue_.push(_callbackPtr);
		});
	}


	/*！
	开启灾备&cache功能后调用
	**/
	void onTimer(gsf::ModuleID target, gsf::ArgsPtr args)
	{
		int32_t _tag = args->pop_i32();

		if (_tag == TimerType::tt_command) {

			redisPtr_->execCommand();
			mailboxPtr_->dispatch(timerM_, eid::timer::delay_milliseconds, gsf::makeArgs(TimerType::tt_command, execDelay_));
		}
		else if (_tag == TimerType::tt_rewrite) {

			redisPtr_->execRewrite();
			mailboxPtr_->dispatch(timerM_, eid::timer::delay_milliseconds, gsf::makeArgs(TimerType::tt_rewrite, rewriteDelay_));
		}
	}

private:

	bool useCache_ = false;

	enum TimerType {
		tt_rewrite,
		tt_command,
	};

	const int32_t rewriteDelay_ = 1000 * 60 * 10;
	const int32_t execDelay_ = 1000;

	gsf::ModuleID timerM_ = gsf::ModuleNil;

	gsf::modules::RedisPtr redisPtr_ = nullptr;

	struct CallbackInfo
	{
		gsf::ModuleID target_ = gsf::ModuleNil;
		gsf::ArgsPtr args_ = nullptr;
	};
	typedef std::queue<CallbackInfo *> CallbackQueue;
	
	CallbackQueue queue_;

	gsf::modules::MysqlPtr mysqlPtr_ = nullptr;
};