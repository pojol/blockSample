
create_sql = string.format("create table if not exists Entity(%s%s%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "mp INT NOT NULL,"
	, "lv INT NOT NULL,"
	, "gold INT NOT NULL,"
	, "loginTime INT NOT NULL")

entity_map = {}

EntityState = {
	undefine = 0,
	init = 1,
	usable = 2,
	unusable = 3,
}

local entityMgr = {
	
	state_ = EntityState.undefine,

	init = function() end,

	entityCreate = function() end,
	entityLoad = function(accountid) end,

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

entityMgr.init = function(self)

	self.state_ = EntityState.init

	-- 创建或更新数据集
	rpc(eid.distributed.mysql_query, module_id, {module_id, create_sql}, function(res, progress, succ) 
		
		if succ == true then
			self.state_ = EntityState.usable
		else 
			logWarn('entityMgr', res[1])
		end	
	
	end)
end

-- 查询entity是否存在, 如果存在则获取不存在则创建
entityMgr.entityLoad = function(self, accountid)

	--sql = string.format("select * from Account where account=%s;", tostrng(accountid))
	sql = string.format("select * from Account where account='%d';", accountid)

	rpc(eid.distributed.mysql_query, module_id, {module_id, sql}, function(res, progress, succ)

		if succ == true and progress ~= -1 then
			_account = res[1]
			_pwd = res[2]
			_entityid = res[3]
			_time = res[4]

			if _entityid == 0 then
				-- create entity
				self.entityCreate()
			else
			end

		elseif succ == false then
			logWarn('entityMgr', res[1])
		end
		
	end)

end

entityMgr.entityCreate = function(self)
	entity = require "entity"
	print(entity.create_sql)

	local pack = Args.new()
	pack:push_i32(module_id)
	pack:push_string(entity.create_sql)
--[[
	rpc(eid.distributed.mysql_query, module_id, pack:pop_block(0, pack:get_pos()), function(buf, len, progress, cbResult)
		print(progress, cbResult)
		local unpack = Args.new(buf, len)
		if true == cbResult then
			print("create entity succ!")


		else
			logWarn('entityMgr', unpack:pop_string())
		end

	end)
]]--
	entity:update("hp", 101)
end

return entityMgr