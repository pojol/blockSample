module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
mysqlM_ = 0

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	logM_ = APP:getModule("LogModule")
	INFO_LOG("mysql", "log : " .. logM_)

	mysqlM_ = APP:getModule("MysqlProxyModule")
	INFO_LOG("mysql", "dbProxy : " .. mysqlM_)
end

mysql_ip = "127.0.0.1"
mysql_usr = "root"
mysql_pwd = "root"
mysql_name = "Tmp"
mysql_port = 3306

module.init = function()

	dispatch(mysqlM_, eid.db_proxy.mysql_connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port))

	listen(module_id, eid.db_proxy.mysql_callback, function(args)
		DEBUG_LOG("mysql", "callback", args)
	end)

	_createSql = string.format("create table if not exists Entity(%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"name VARCHAR(32) NOT NULL,"
		,"loginTime INT NOT NULL")

	dispatch(mysqlM_, eid.distributed.mysql_query, evpack:mysql_query(module_id, _createSql))

	_addSql = string.format("insert into Entity (id, name, loginTime) values (%d, %s, %d);"
		,0
		,"'test'"
		,os.time())
	
	INFO_LOG("mysql", "sql", _addSql)
	dispatch(mysqlM_, eid.distributed.mysql_query, evpack:mysql_query(module_id, _addSql))
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end