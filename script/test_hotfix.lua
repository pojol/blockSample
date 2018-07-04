module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
mysqlM_ = 0

mysql_ip = "47.96.147.176"
mysql_usr = "root"
mysql_pwd = "alvinlh"
mysql_name = "Tmp"
mysql_port = 3306

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	logM_ = APP:getModule("LogModule")
	INFO_LOG("test", "log : " .. logM_)

	mysqlM_ = APP:getModule("MysqlProxyModule")
	INFO_LOG("mysql", "dbProxy : " .. mysqlM_)
end


module.init = function()
	INFO_LOG("test", "hello, world!")

	dispatch(mysqlM_, eid.dbProxy.connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port))
end


module.execute = function()

end


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

module.shut = function()

	local _add = {}

	for k, val in pairs(entity.property) do
		table.insert(_add, k)
		table.insert(_add, val)
	end

	dispatch(mysqlM_, eid.dbProxy.insert, evpack:dbInsert(sqloper.insert, "Entity", _add))

end


module.after_shut = function()
end