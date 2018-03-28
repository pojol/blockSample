module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
mysqlM_ = 0
local protobuf_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"
	protobuf_ = require "protobuf"

	logM_ = APP:getModule("LogModule")
	INFO_LOG("mysql", "log : " .. logM_)

	mysqlM_ = APP:getModule("MysqlProxyModule")
	INFO_LOG("mysql", "dbProxy : " .. mysqlM_)
end

mysql_ip = "47.96.147.176"
mysql_usr = "root"
mysql_pwd = "alvinlh"
mysql_name = "Tmp"
mysql_port = 3306

local entity = {
	property = {
		id = 0,
        name = "''",
        hp = 100,
        lv = 1,
        loginTime = 0
	}
}

sqloper = {
	query = 1,
	insert = 2,
	load = 3,
}


module.init = function()

	dispatch(mysqlM_, eid.dbProxy.connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port))

	_createSql = string.format("create table if not exists Entity(%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"name VARCHAR(32) NOT NULL,"
		,"hp INT NOT NULL,"
		,"lv INT NOT NULL,"
		,"loginTime INT NOT NULL")

	listen(eid.dbProxy.callback, function(args)
		if args[1] == sqloper.query then
			DEBUG_LOG("mysql", "query callback", args)
		elseif args[1] == sqloper.insert then
			DEBUG_LOG("mysql", "insert callback", args)
		elseif args[1] == sqloper.load then
			DEBUG_LOG("mysql", "load callback", args)
		end
	end)

	
	dispatch(mysqlM_, eid.dbProxy.execSql, evpack:dbQuery(sqloper.query, _createSql))

	local _add = {}

	for k, val in pairs(entity.property) do
		table.insert(_add, k)
		table.insert(_add, val)
	end

	
	dispatch(mysqlM_, eid.dbProxy.insert, evpack:dbInsert(sqloper.insert, "Entity", _add))


	--[[ load all
		dispatch(mysqlM_, eid.dbProxy.load, evpack:dbLoad("Entity", 3), function(args)
			DEBUG_LOG("mysql", "load entity", args)
		end)
	]]--

	dispatch(mysqlM_, eid.dbProxy.load, evpack:dbLoad(sqloper.load, "Entity", 0))

	--[[ update entity
		_entityID = 1
		_entityDirty = {
			"name",			-- key
			"'test1'"		-- value
		}
		dispatch(mysqlM_, eid.mysql_update, evpack:entity_update("Entity", _entityID, _entityDirty))
	]]--
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end