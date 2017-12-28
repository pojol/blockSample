#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>


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

uint8_t ToValueType(enum_field_types mysqlType)
{
	switch (mysqlType)
	{
	case FIELD_TYPE_TIMESTAMP:
	case FIELD_TYPE_DATE:
	case FIELD_TYPE_TIME:
	case FIELD_TYPE_DATETIME:
	case FIELD_TYPE_YEAR:
	case FIELD_TYPE_STRING:
	case FIELD_TYPE_VAR_STRING:
	case FIELD_TYPE_BLOB:
	case FIELD_TYPE_SET:
	case FIELD_TYPE_NULL:
		return gsf::at_string;
	case FIELD_TYPE_TINY:
	case FIELD_TYPE_SHORT:
	case FIELD_TYPE_ENUM:
	case FIELD_TYPE_LONG:
		return gsf::at_int32;
	case FIELD_TYPE_INT24:
	case FIELD_TYPE_LONGLONG:
		return gsf::at_int64;
	case FIELD_TYPE_DECIMAL:
	case FIELD_TYPE_FLOAT:
		return gsf::at_float;
	case FIELD_TYPE_DOUBLE:
		return gsf::at_double;
	default:
		return gsf::at_eof;
	}
}

std::pair<enum_field_types, char> ToMySqlType(uint8_t cppType)
{
	std::pair<enum_field_types, char> ret(MYSQL_TYPE_NULL, 0);

	switch (cppType)
	{
	case gsf::at_uint8:
		ret.first = MYSQL_TYPE_TINY;
		ret.second = 1;
		break;
	case gsf::at_int8:
		ret.first = MYSQL_TYPE_TINY;
		ret.second = 0;
		break;
	case gsf::at_uint16:
		ret.first = MYSQL_TYPE_SHORT;
		ret.second = 1;
		break;
	case gsf::at_int16:
		ret.first = MYSQL_TYPE_SHORT;
		ret.second = 0;
		break;
	case gsf::at_uint32:
		ret.first = MYSQL_TYPE_LONG;
		ret.second = 1;
		break;
	case gsf::at_int32:
		ret.first = MYSQL_TYPE_LONG;
		ret.second = 0;
		break;
	case gsf::at_uint64:
		ret.first = MYSQL_TYPE_LONGLONG;
		ret.second = 1;
		break;
	case gsf::at_int64:
		ret.first = MYSQL_TYPE_LONGLONG;
		ret.second = 0;
		break;
	case gsf::at_float:
		ret.first = MYSQL_TYPE_FLOAT;
		ret.second = 0;
		break;
	case gsf::at_double:
		ret.first = MYSQL_TYPE_DOUBLE;
		ret.second = 0;
		break;
	case gsf::at_string:
		ret.first = MYSQL_TYPE_STRING;
		ret.second = 0;
		break;
	}
	return ret;
}

struct SqlStmt
{
	SqlStmt()
		: result(nullptr)
		, stmt(nullptr)
	{}

	~SqlStmt()
	{
		if (stmt) {
			mysql_free_result(result);
			mysql_stmt_close(stmt);
		}
	}

	uint32_t params;
	uint32_t columns;

	bool is_query;
	bool is_prepared;

	std::string sql = "";

	MYSQL_RES*		result;
	MYSQL_STMT*		stmt;
};

typedef std::shared_ptr<SqlStmt> SqlStmtPtr;

MYSQL * mysqlPtr;

std::unordered_map<std::string, SqlStmtPtr> prepared_stmt_map;

void perpare(const std::string &sql, SqlStmtPtr &stmtPtr)
{
	auto itr = prepared_stmt_map.find(sql);
	if (itr != prepared_stmt_map.end()) {
		stmtPtr = itr->second;
		return;
	}

	do {

		stmtPtr = std::make_shared<SqlStmt>();
		stmtPtr->sql = sql;

		stmtPtr->stmt = mysql_stmt_init(mysqlPtr);
		if (nullptr == stmtPtr->stmt) {
			std::cout << "stmt init fail" << std::endl;
			break;
		}

		if (mysql_stmt_prepare(stmtPtr->stmt, sql.c_str(), (unsigned long)sql.size())) {
			std::cout << mysql_stmt_error(stmtPtr->stmt) << std::endl;
			break;
		}

		stmtPtr->params = mysql_stmt_param_count(stmtPtr->stmt);
		stmtPtr->result = mysql_stmt_result_metadata(stmtPtr->stmt);

		std::string sqlop = sql.substr(0, 6);
		if (nullptr == stmtPtr->result && (strcmp(sqlop.c_str(), "SELECT") == 0 || strcmp(sqlop.c_str(), "select") == 0)) {
			std::cout << mysql_stmt_error(stmtPtr->stmt) << std::endl;
			break;
		}

		if (stmtPtr->result) {
			stmtPtr->is_query = true;
			stmtPtr->columns = mysql_num_fields(stmtPtr->result);
		}
		prepared_stmt_map.emplace(sql, stmtPtr);
		return;

	} while (0);

	stmtPtr.reset();
}

void ExecuteStmt(const SqlStmtPtr& stmt, const gsf::ArgsPtr &args)
{
	if (stmt->params != args->get_params())
	{
		std::cout << " enough params " << std::endl;
		return;
	}

	std::vector<MYSQL_BIND> mysqlBinds;
	auto _tag = args->get_tag();
	while (0 != _tag) 
	{
		auto mt = ToMySqlType(_tag);
		mysqlBinds.emplace_back(MYSQL_BIND());
		auto& Param = mysqlBinds.back();
		memset(&Param, 0, sizeof(MYSQL_BIND));
		Param.buffer_type = mt.first;
		Param.is_null = 0;
		Param.is_unsigned = mt.second;
		Param.length = 0;

		switch (_tag)
		{
		case gsf::at_int8:
			auto _i8 = args->pop_i8();
			Param.buffer = (void*)&_i8;
		case gsf::at_uint8:
			auto _ui8 = args->pop_ui16();
			Param.buffer = (void *)&_ui8;
		case gsf::at_int16:
			//...

			Param.buffer_length = (unsigned long)0;
			break;
		case gsf::at_string:
			std::string _s = args->pop_string();
			Param.buffer = (void*)_s.data();
			Param.buffer_length = (unsigned long)_s.size();
			break;
		}

		_tag = args->get_tag();
	}

	// bind input arguments
	if (mysql_stmt_bind_param(stmt->stmt, mysqlBinds.data()))
	{
		std::cout << mysql_stmt_error(stmt->stmt) << std::endl;
		return;
	}

	if (mysql_stmt_execute(stmt->stmt))
	{
		std::cout << mysql_stmt_error(stmt->stmt) << std::endl;
		return;
	}
}

void init()
{
	auto mysqlInit = mysql_init(nullptr);
	if (nullptr == mysqlInit) {
		std::cout << "err" << std::endl;
	}

	mysqlPtr = mysql_real_connect(mysqlInit, "192.168.50.130", "root", "root", "Logs233", 3306, nullptr, 0);
	if (nullptr == mysqlPtr) {
		mysql_close(mysqlPtr);
	}

	
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


	//init();


	//app.regist_module(new TestModule());

	app.run();

	return 0;
}
