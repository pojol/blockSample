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

#include <fmt/format.h>


struct Entity
{
	uint32_t id_ = 0;

	std::string name_ = "";
	uint32_t hp_ = 100;
	uint32_t mp_ = 100;
	uint32_t lv_ = 1;
	uint32_t gold_ = 300;
	uint64_t loginTime_ = time((time_t*)NULL);

	std::string get_name() { return name_; }
	void set_name(const std::string &name) { name_ = name; dirtyList_.insert("name"); }

	uint32_t get_hp() { return hp_; }
	void set_hp(uint32_t hp) { hp_ = hp; dirtyList_.insert("hp"); }

	gsf::ArgsPtr sql_update()
	{
		auto args = gsf::ArgsPool::get_ref().get();

		args->push("Entity");

		for each (auto dirty in dirtyList_)
		{
			if (dirty == "name") {
				args->push("name");
				args->push(name_);
			}
			if (dirty == "hp") {
				args->push("hp");
				args->push(hp_);
			}
			if (dirty == "mp") {
				args->push("mp");
				args->push(mp_);
			}
			if (dirty == "lv") {
				args->push("lv");
				args->push(lv_);
			}
			if (dirty == "gold") {
				args->push("gold");
				args->push(gold_);
			}
			if (dirty == "loginTime") {
				args->push("loginTime");
				args->push(loginTime_);
			}
		}

		dirtyList_.clear();

		if (args->get_params() > 1) {
			return args;
		}
		else {
			return nullptr;
		}
	}

private:
	std::set<std::string> dirtyList_;
};

typedef std::shared_ptr<Entity> EntityPtr;

class EntityMgrModule
	: public gsf::IEvent
	, public gsf::Module
{
public:

	EntityMgrModule()
		: Module("EntityMgrModule")
	{}

	virtual ~EntityMgrModule() {}

	void before_init() override
	{
		timer_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("TimerModule"))->pop_moduleid();
		assert(timer_m_ != gsf::ModuleNil);
	}

	void init() override
	{
		rpc("DBProxyServerModule", eid::distributed::db_create, gsf::make_args(sql_create()), [&](const gsf::ArgsPtr &args, bool result) {
			
		});
	}

	void shut() override
	{

	}

protected:
	gsf::ArgsPtr timeArrive(const gsf::ArgsPtr &args)
	{
		auto _tid = args->pop_timerid();

		if (_tid == sql_update_timer_) {
		
			for (auto &entity : entity_map_)
			{
				gsf::ArgsPtr su_args = entity.second->sql_update();
				if (nullptr != su_args) {
					rpc("DBProxyServerModule", eid::distributed::db_update, su_args);
				}
			}
		}

		return nullptr;
	}

	std::string sql_create()
	{
		std::string _create_sql = fmt::format("create table if not exists Entity({}{}{}{}{}{}{}) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=tuf8_bin;"
			, "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
			, "name VARCHAR(32) NOT NULL,"
			, "hp INT NOT NULL,"
			, "mp INT NOT NULL,"
			, "lv INT NOT NULL,"
			, "gold INT NOT NULL,"
			, "loginTime INT NOT NULL");

		return _create_sql;
	}

private:

	gsf::ModuleID timer_m_ = gsf::ModuleNil;

	gsf::TimerID sql_update_timer_ = gsf::TimerNil;

private:

	std::unordered_map<uint32_t, EntityPtr> entity_map_;

};

class DBClientModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	DBClientModule()
		: Module("DBClientModule")
	{}

	virtual ~DBClientModule() {}

	void before_init() override
	{
		entity_mgr_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("EntityMgrModule"))->pop_moduleid();
		assert(entity_mgr_m_ != gsf::ModuleNil);
	}

	void init() override
	{

	}

	void shut() override
	{

	}

private:
	gsf::ModuleID db_connector_m_ = gsf::ModuleNil;
	gsf::ModuleID entity_mgr_m_ = gsf::ModuleNil;
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

	app.regist_module(new EntityMgrModule);

	app.run();

	return 0;
}
