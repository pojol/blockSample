#include "db_avatar.h"


void db_avatar::init()
{
	//auto _eid = make_event_id(eid::db_proxy::update);
	listen(this, get_module_id(), std::bind(&db_avatar::event_update, this, std::placeholders::_1));
	listen(this, get_module_id(), std::bind(&db_avatar::event_remove, this, std::placeholders::_1));
	listen(this, get_module_id(), std::bind(&db_avatar::event_load, this, std::placeholders::_1));
}

void db_avatar::execute()
{

}

void db_avatar::shut()
{

}


gsf::ArgsPtr db_avatar::event_update(const gsf::ArgsPtr &args)
{
	google::protobuf::Message *msg;
	Avatar _avatar;

	auto descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("Avatar");
	if (!descriptor) {
		assert(descriptor != nullptr);
		return nullptr;
	}
	
	auto reflection = _avatar.GetReflection();
	if (!reflection) {
		assert(reflection != nullptr);
		return nullptr;
	}

	for (int i = 0; i < descriptor->field_count(); ++i)
	{
		auto field = descriptor->field(i);
		if (!field) {
			assert(field != nullptr);
			continue;
		}

		field->name();
	}

	return nullptr;
}

gsf::ArgsPtr db_avatar::event_remove(const gsf::ArgsPtr &args)
{
	return nullptr;
}

gsf::ArgsPtr db_avatar::event_load(const gsf::ArgsPtr &args)
{
	return nullptr;
}