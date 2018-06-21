#pragma once

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

#include <utils/timer.hpp>
#include <utils/logger.hpp>

#include <luaAdapter/luaAdapter.h>

#include <dbProxy/mysqlConnect.h>
#include <dbProxy/redisConnect.h>

#include "Avatar.pb.h"

//! auto gen file

class DBEntityModule
	: public block::Module
{
public:

	DBEntityModule()
		: block::Module("DBEntityModule")
	{

	}

protected:
	void before_init() override
	{
		using namespace std::placeholders;
		
		listen(block::event::db_connect
			, std::bind(&DBEntityModule::eInit, this, std::placeholders::_1, std::placeholders::_2));

		listen(block::event::db_execSql
			, std::bind(&DBEntityModule::eExecSql, this, std::placeholders::_1, std::placeholders::_2));

		listen(block::event::db_load
			, std::bind(&DBEntityModule::eLoad, this, std::placeholders::_1, std::placeholders::_2));

		listen(block::event::db_insert
			, std::bind(&DBEntityModule::eInsert, this, std::placeholders::_1, std::placeholders::_2));

		listen(block::event::db_update
			, std::bind(&DBEntityModule::eUpdate, this, std::placeholders::_1, std::placeholders::_2));
	
		mysqlPtr_ = std::make_shared<block::modules::MysqlConnect>();
	}

	void init() override
	{
	}

	void execute() override
	{
		if (useCache_) {
			redisPtr_->run();
		}

		while (!queue_.empty())
		{
			auto _callback = queue_.front();

			dispatch(_callback->target_, block::event::db_callback, std::move(_callback->args_));

			delete _callback;
			_callback = nullptr;

			queue_.pop();
		}
	}

	void shut() override
	{
		//! 清空redis
		redisPtr_->flush_redis_handler();
	}

private:
	
	void eInit(block::ModuleID target, block::ArgsPtr args)
	{
		auto _host = args->pop_string();	//host
		auto _user = args->pop_string(); //user
		auto _password = args->pop_string(); //password
		auto _dbName = args->pop_string(); //database
		auto _port = args->pop_i32();	//port
		auto _useCache = args->pop_bool();

		if (!mysqlPtr_->init(_host, _port, _user, _password, _dbName)) {
			ERROR_LOG("MysqlProxy init fail!");
			return;
		}

		useCache_ = _useCache;
		if (useCache_) {
			redisPtr_ = std::make_shared<block::modules::RedisConnect<test::Avatar>>();
			redisPtr_->init();

			TIMER.delay(block::utils::delay_milliseconds(rewriteDelay_), std::bind(&DBEntityModule::onTimer, this));
		}
	}

	/*!
	获取一个实例
	**/
	void eLoad(block::ModuleID target, block::ArgsPtr args)
	{
		auto _id = args->pop_i32();

		if (useCache_) {

			//! 先尝试从缓存中读取
			std::string _buf = redisPtr_->get("entity", _id);
			if (_buf != "") {

				std::cout << "----redis cache " << _buf << std::endl;

				auto _callbackPtr = new CallbackInfo();
				_callbackPtr->args_ = block::makeArgs(int(block::event::db_load), true, 1, 1, _id, _buf);
				_callbackPtr->target_ = target;
				queue_.push(_callbackPtr);

				return;
			}
		}

		std::string _sql = "select * from Entity where id = '" + std::to_string(_id) + "';";

		mysqlPtr_->execSql(target, block::event::db_load, _sql, [&](block::ModuleID _target, block::ArgsPtr args) {

			if (useCache_) {
				auto _targs = block::ArgsPool::get_ref().get();
				_targs->importBuf(args->exportBuf());

				_targs->pop_i32();
				auto _succ = _targs->pop_bool();
				if (_succ) {
					_targs->pop_i32();
					_targs->pop_i32();
					auto _id = _targs->pop_i32();
					auto _buf = _targs->pop_string();

					//...
					redisPtr_->push("entity", _id, _buf);
				}
			}

			auto _callbackPtr = new CallbackInfo();
			_callbackPtr->args_ = std::move(args);
			_callbackPtr->target_ = _target;
			queue_.push(_callbackPtr);
		});
	}

	/*!
	创建一个实例
	**/
	void eInsert(block::ModuleID target, block::ArgsPtr args)
	{
		auto _buf = args->pop_string();
		if (mysqlPtr_->insert("insert into Entity values(?, ?);", _buf.c_str(), _buf.length())) {
			
			mysqlPtr_->execSql(target, block::event::db_insert, "select last_insert_id()", [&](block::ModuleID _target, block::ArgsPtr args) {
				auto _callbackPtr = new CallbackInfo();
				_callbackPtr->args_ = std::move(args);
				_callbackPtr->target_ = _target;
				queue_.push(_callbackPtr);
			});
		}
	}

	/*!
	更新一个实例
	**/
	void eUpdate(block::ModuleID target, block::ArgsPtr args)
	{
		auto _id = args->pop_i32();
		auto _buf = args->pop_string();

		if (useCache_) {
			redisPtr_->push("entity", _id, _buf);
		}
		else {
			update(std::to_string(_id), _buf);
		}
	}

	void update(const std::string &id, const std::string &buf)
	{
		mysqlPtr_->update("update Entity set dat = ? where id = '" + id + "';", buf.c_str(), buf.length());
	}

	/*!
	执行一条sql语句
	**/
	void eExecSql(block::ModuleID target, block::ArgsPtr args)
	{
		std::string queryStr = args->pop_string();

		using namespace std::placeholders;

		mysqlPtr_->execSql(target, block::event::db_execSql, queryStr, [&](block::ModuleID _target, block::ArgsPtr args) {
			auto _callbackPtr = new CallbackInfo();
			_callbackPtr->args_ = std::move(args);
			_callbackPtr->target_ = _target;
			queue_.push(_callbackPtr);
		});
	}


	/*！
	开启灾备&cache功能后调用
	**/
	void onTimer()
	{
		auto _vec = redisPtr_->getAll();
		for each(auto kv in _vec)
		{
			if (kv.first != 0) {
				update(std::to_string(kv.first), kv.second);
			}
		}

		TIMER.delay(block::utils::delay_milliseconds(rewriteDelay_), std::bind(&DBEntityModule::onTimer, this));
	}

private:

	bool useCache_ = false;

	enum TimerType {
		tt_rewrite,
		tt_command,
	};

	const int32_t rewriteDelay_ = 1000 * 60 * 1;

	block::ModuleID timerM_ = block::ModuleNil;

	std::shared_ptr<block::modules::RedisConnect<test::Avatar>> redisPtr_ = nullptr;

	struct CallbackInfo
	{
		block::ModuleID target_ = block::ModuleNil;
		block::ArgsPtr args_ = nullptr;
	};
	typedef std::queue<CallbackInfo *> CallbackQueue;
	
	CallbackQueue queue_;

	block::modules::MysqlPtr mysqlPtr_ = nullptr;
};