#pragma once

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

	Entity()
	{

	}

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

		client_m_ = dispatch(eid::app_id, eid::get_module, gsf::make_args("DBClientModule"))->pop_moduleid();
		assert(client_m_ != gsf::ModuleNil);
	}

	void init() override
	{
		listen(this, 10002, [&](const gsf::ArgsPtr &args) {
		
			dispatch(client_m_, 10003, gsf::make_args(sql_create()));
			//dispatch(client_m_, 10003, gsf::make_args())
			sql_update_timer_ = dispatch(timer_m_, eid::timer::delay_milliseconds, gsf::make_args(1000))->pop_timerid();

			return nullptr;
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
					dispatch(client_m_, 10004, su_args);
				}
			}
		}

		return nullptr;
	}

	std::string sql_create()
	{
		std::string _create_sql = fmt::format("create table if not exists Entity({}{}{}{}{}{}{}) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
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
	gsf::ModuleID client_m_ = gsf::ModuleNil;

	gsf::TimerID sql_update_timer_ = gsf::TimerNil;

private:

	std::unordered_map<uint32_t, EntityPtr> entity_map_;

};