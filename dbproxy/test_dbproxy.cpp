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



#include <log/log.h>

#include <iostream>

#include "db_avatar.h"

#include "mysql.h"

uint64_t get_system_tick()
{
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

class TestModule
	: public gsf::Module
	, public gsf::IEvent
{
public:

	TestModule()
		: Module("TestModule")
	{}

	google::protobuf::Message * find_message(const std::string &key1, const std::string &key2)
	{
		
	}

	google::protobuf::Message * find_message(const std::string &key1)
	{
		
	}

	void init()
	{
		// 目标数据对象
		Avatar _avatar_b;
		_avatar_b.set_hp(11);
		_avatar_b.set_gold(100);

		char buf[256];

		auto _beg = get_system_tick();

		for (int i = 0; i < 100000; ++i)
		{
			// 源数据对象
			Avatar _avatar_a;
			_avatar_a.set_hp(10);
			_avatar_a.set_mp(10);

			// 局部更新
			_avatar_a.SerializeToArray(buf, 256);

			Avatar _avatar_t;
			_avatar_t.ParseFromArray(buf, 256);

			// 差异覆盖
			_avatar_b.MergeFrom(_avatar_t);
		}

		auto _end = get_system_tick();
		std::cout << "consume " << _end - _beg << " ms"<< std::endl;


		//_avatar_b.MergeFrom(_avatar_a);
		//std::cout << _avatar_b.hp() << " " << _avatar_b.mp() << " " << _avatar_b.gold() << std::endl;

		/*
		gsf::Args args(std::string("grids"), std::string("pos"), uint32_t(11));

		std::string _table_name = args.pop_string(0);
		std::string _field_name = args.pop_string(1);

		auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(_table_name);
		if (!descriptor) {
			assert(descriptor != nullptr);
			return;
		}

		//! ���redis��û��
		//auto proto = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		//auto msg = proto->New();

		auto reflection = _avatar.GetReflection();
		if (!reflection) {
			assert(reflection != nullptr);
			return;
		}

		for (int i = 0; i < descriptor->field_count(); ++i)
		{
			auto field = descriptor->field(i);
			if (!field) {
				assert(field != nullptr);
				continue;
			}

			if (field->name() == _field_name) {
				switch (field->type())
				{
				case google::protobuf::FieldDescriptor::Type::TYPE_STRING:
					reflection->SetString(&_avatar, field, args.pop_string(2));
					break;
				case google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE:
					for each (auto var in _avatar.grids())
					{
						if (var.pos() == 1) {
							
						}

						
					}
					break;
				}
			}
		}

		for each (auto var in _avatar.grids())
		{
			std::cout << var.pos() << std::endl;
		}

		*/
	}
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

	app.regist_module(new TestModule());

	app.run();

	return 0;
}
