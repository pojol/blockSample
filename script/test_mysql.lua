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

	addr = io.open("C:/github/blockSample/depend/proto/lua/Avatar.pb","rb")
	buffer = addr:read "*a"
	addr:close()

	protobuf_.register(buffer)

	self:logInfo("mysql" .. "log : " .. logM_)

	dbEntityM_ = APP:getModule("DBEntityModule")
	self:logInfo("mysql" .. "dbEnityModule : " .. dbEntityM_)
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

function noCacheTest()
	self:logInfo("[BEGIN] [M]test_mysql [T]noCacheTest")

	listen(eid.dbProxy.callback, function(args)
		if args[1] == eid.dbProxy.execSql then
			self:logInfo("[Callback] execSql res : " .. dumpStr(args))

			_buf = protobuf_.encode("test.Avatar", entity.property)
			dispatch(dbEntityM_, eid.dbProxy.insert, evpack:dbInsert(_buf))

		elseif args[1] == eid.dbProxy.insert then
			self:logInfo("[Callback] insert res : " .. dumpStr(args))

			entity.property.name = "hello"
			_buf = protobuf_.encode("test.Avatar", entity.property)
			dispatch(dbEntityM_, eid.dbProxy.update, evpack:dbUpdate(args[5], _buf))

			dispatch(dbEntityM_, eid.dbProxy.load, evpack:dbLoad(args[5]))

		elseif args[1] == eid.dbProxy.load then
			self:logInfo("[Callback] load res : " .. dumpStr(args))

			_avatar = protobuf_.decode("test.Avatar", args[6])
			self:logInfo("[Callback] entity : " .. dumpStr(_avatar))
		end
	end)

	-- 连接mysql
	dispatch(dbEntityM_, eid.dbProxy.connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port, false))

	-- 创建表
	_createEntity = string.format("create table if not exists Entity(%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"dat blob NOT NULL")

	dispatch(dbEntityM_, eid.dbProxy.execSql, evpack:dbQuery(_createEntity))

	self:logInfo("[END] [M]test_mysql [T]noCacheTest")
end

function cacheTest()

	listen(eid.dbProxy.callback, function(args)
		
		if args[1] == eid.dbProxy.load then

			if args[2] == true then -- 打印这个entity, 并更新一下				
				_avatar = protobuf_.decode("test.Avatar", args[6])
				self:logInfo("mysql" .. "load" .. "entity->" .. _avatar)

				entity.property = _avatar
				entity.property.id = args[5]
				entity.property.name = "name"
				-- update
				_buf = protobuf_.encode("test.Avatar", entity.property)
				dispatch(dbEntityM_, eid.dbProxy.update, evpack:dbUpdate(args[5], _buf))

			else -- 如果空则insert一个entity
				-- insert
				_buf = protobuf_.encode("test.Avatar", entity.property)
				dispatch(dbEntityM_, eid.dbProxy.insert, evpack:dbInsert(_buf))
			end

		elseif args[1] == eid.dbProxy.insert then
			-- 打印 entity id
			if args[2] == true then
			
				self:logDebug("mysql" .. "insert" .. "id->" .. dumpStr(args[5]))
				-- 加载刚刚创建的那个entity
				dispatch(dbEntityM_, eid.dbProxy.load, evpack:dbLoad(args[5]))
			end
		end
	
	end)

	-- 连接mysql
	dispatch(dbEntityM_, eid.dbProxy.connect, evpack:mysql_connect(mysql_ip, mysql_usr, mysql_pwd, mysql_name, mysql_port, true))

	-- 建立entity table
	_createEntity = string.format("create table if not exists Entity(%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
		,"id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
		,"dat blob NOT NULL")

	dispatch(dbEntityM_, eid.dbProxy.execSql, evpack:dbQuery(_createEntity))

	-- load entity 
	dispatch(dbEntityM_, eid.dbProxy.load, evpack:dbLoad(1))

end

module.init = function()

	noCacheTest()
	-- cacheTest()

end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end