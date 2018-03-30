module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
dbEntityM_ = 0
local protobuf_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"
	protobuf_ = require "protobuf"

	addr = io.open("C:/github/gsf_sample/depend/proto/lua/Avatar.pb","rb")
	buffer = addr:read "*a"
	addr:close()

	protobuf_.register(buffer)

	logM_ = APP:getModule("LogModule")
	INFO_LOG("mysql", "log : " .. logM_)

	dbEntityM_ = APP:getModule("DBEntityModule")
	INFO_LOG("mysql", "dbEnityModule : " .. dbEntityM_)
end

mysql_ip = "47.96.147.176"
mysql_usr = "root"
mysql_pwd = "alvinlh"
mysql_name = "Tmp"
mysql_port = 3306

local entity = {
	property = {
		id = 0,
        name = "",
        hp = 100,
        lv = 1,
        loginTime = 0
	}
}

module.init = function()

	dispatch(dbEntityM_, eid.dbProxy.connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port))

	_createEntity = string.format("create table if not exists Entity(%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"dat blob NOT NULL")

	_createLog = string.format("create table if not exists Log_EntityBehavior(%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"name VARCHAR(256) NOT NULL,"
		,"time INT NOT NULL,"
		,"behavior INT NOT NULL")

	listen(eid.dbProxy.callback, function(args)
		if args[1] == eid.dbProxy.execSql then
			DEBUG_LOG("mysql", "query callback", args)
		elseif args[1] == eid.dbProxy.insert then
			DEBUG_LOG("mysql", "insert callback", args)
		elseif args[1] == eid.dbProxy.load then

			_avatar = protobuf_.decode("test.Avatar", args[6])
			DEBUG_LOG("mysql", "load", "oper->", args[1]
				, "succ->", args[2]
				, "total->", args[3]
				, "progress->", args[4]
				, "id->", args[5]
				, "entity->", _avatar)
		end
	end)
	
	--dispatch(dbEntityM_, eid.dbProxy.execSql, evpack:dbQuery(_createEntity))
	--dispatch(dbEntityM_, eid.dbProxy.execSql, evpack:dbQuery(_createLog))
	
	-- insert
	--_buf = protobuf_.encode("test.Avatar", entity.property)
	--dispatch(dbEntityM_, eid.dbProxy.insert, evpack:dbInsert(_buf))

	-- load
	dispatch(dbEntityM_, eid.dbProxy.load, evpack:dbLoad(1))

	-- update
	--entity.property.name = "hello"
	--_buf = protobuf_.encode("test.Avatar", entity.property)
	--dispatch(dbEntityM_, eid.dbProxy.update, evpack:dbUpdate(1, _buf))
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end