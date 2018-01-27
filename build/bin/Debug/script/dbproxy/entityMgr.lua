
create_sql = string.format("create table if not exists Entity(%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "lv INT NOT NULL,"
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

	entity_create = function() end,
	entity_init = function(entity_id) end,
	entity_load = function(account_id) end,

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
	rpc(eid.distributed.mysql_query, module_id, evpack:mysql_query(module_id, create_sql), function(res, progress, succ) 
		if succ == true then
			self.state_ = EntityState.usable
		else 
			logWarn('entityMgr', res[1])
		end	
	
	end)
end

-- 查询entity是否存在, 如果存在则获取不存在则创建
entityMgr.entity_load = function(self, accountid)
	print("load entity : " .. accountid)

	sql = string.format("select * from Account where id='%d';", accountid)
	rpc(eid.distributed.mysql_query, module_id, evpack:mysql_query(module_id, sql), function(res, progress, succ)

		if succ == true and progress ~= -1 then
			_account = res[1]
			_pwd = res[2]
			_entityid = res[3]
			_time = res[4]
			
			print("account: " .. _account .. " pwd " .. _pwd .. " entity " .. "time" .. _time)

			if _entityid == 0 then
				print("create entity")
				-- create entity
				self:entity_create(_account)
			else
				print("init entity")
				self:entity_init(_entityid)
			end

		elseif succ == false then
			logWarn('entityMgr', res[1])
		end
		
	end)

end

entityMgr.entity_init = function(self, entity_id)

	_sql = string.format("select * from Entity where id = '%d';", entity_id);
	rpc(eid.distributed.mysql_query, module_id, evpack:mysql_query(module_id, _sql), function(res, progress, succ)
	
		if true == succ and progress > 0 then

			entity_map[entity_id] = deep_copy(require "entity")
			
			entity_map[entity_id]:init(res)
			
			-- 这里模拟下entity的更新行为
			entity_map[entity_id]:setProperty("hp", 50)
			entity_map[entity_id]:update()

		elseif true ~= succ then
			logWarn('entityMgr', res[1])
		end
	
	end)

end

entityMgr.entity_create = function(self, account)
	entity = require "entity"
	_sql = entity:create_sql()
	print(_sql)

	rpc(eid.distributed.mysql_query, module_id, evpack:mysql_query(module_id, _sql), function(res, progress, succ)
		if true == succ and progress > 0 then
			print("id : " .. res[1])

			-- 通用的更新流程看 entity:update  这里懒得建表了
			_updatesql = string.format("update Account set entityID='%d' where id='%d'", account, res[1]) 
			rpc(eid.distributed.mysql_query, module_id, evpack:mysql_query(module_id, _updatesql), nil)

		elseif true ~= succ then
			logWarn('entityMgr', res[1])
		end

	end)

end

return entityMgr
