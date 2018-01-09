
create_sql = string.format("create table if not exists Entity(%s%s%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "mp INT NOT NULL,"
	, "lv INT NOT NULL,"
	, "gold INT NOT NULL,"
	, "loginTime INT NOT NULL")

--[[
new_create_sql = string.format("create table if not exists Entity(%s%s%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "mp INT NOT NULL,"
	, "lv INT NOT NULL,"
	, "gold INT NOT NULL,"
	, "exp INT NOT NULL,"
	, "loginTime INT NOT NULL")
]]--

entity_map = {}

local entityMgr = {
	
	init = function() end,

	entityCreate = function() end,
	entityLoad = function() end,

}

-- 临时测试数据
--[[
	Account
	+---------+-----+----------+------------+
	| account | pwd | entityID | registTime |
	+---------+-----+----------+------------+
	| test    | 111 |        0 | 1515491087 |
	+---------+-----+----------+------------+
]]

entityMgr.init = function()

	-- 创建或更新数据集
	local pack = Args.new()
	pack:push_string(create_sql)
	rpc(eid.db_proxy.mysql_query, pack:pop_block(0, pack:get_pos()), function(buf, len, cbResult)
	
		if cbResult == true then
			local unpack = Args.new(buf, len)
			_bres = unpack:pop_bool()
			if _bres == false then
				_msg = unpack:pop_string()
				logWarn('entityMgr', 'create entity table fail : ' .. _msg)
			end

		end

	end)
end

-- 查询entity是否存在, 如果存在则获取不存在则创建
entityMgr.entityLoad = function()

end

entityMgr.entityCreate = function()
	
end

return entityMgr