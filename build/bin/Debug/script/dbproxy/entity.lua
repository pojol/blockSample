
local entity = {

    -- db property
    property = {
        id = 0,
        name = "",
        hp = 100,
        lv = 1,
        loginTime = 0,
    },

    -- 用于标记需要更新到数据库的字段
    dirty = {

    },

    create_sql = string.format("insert into Entity (id,name,hp,mp,lv,gold,loginTime) values (%d,%s,%d,%d,%d,%d,%d);"
        , 110
        , "''"
        , 100
        , 150
        , 1
        , 300
        , 0),

    init = function(stream) end,

    setProperty = function(type, val) end,
    update = function() end,    
}

entity.init = function(stream)

    -- 从数据库请求数据
    

end

entity.setProperty = function(self, type, val)

    if self.property[type] == nil then
        -- warning
        return
    end
    
    self.property[type] = val
    self.dirty[type] = val 
end


entity.update = function(self)

    -- 如果空则跳过
    if next(self.dirty) == nil then
        return
    end

    it = {"entity"}

    for k, v in pairs(self.dirty) do
        table.insert(it, k)
        table.insert(it, v)
    end

    self.dirty = {}

    rpc(eid.distributed.mysql_update, module_id, it, nil)

    --[[
    print(key, val)

    sql = string.format("update Entity set %s=%d where id='%d';", key, val, 110)

    rpc(eid.distributed.mysql_query, module_id, {module_id, sql}, nil)

    rpc(eid.distributed.mysql_select, module_id, {"Entity", 0}, function(res, progress, succ)
        
    end)
    ]]--

    --rpc(eid.distributed.mysql_update, module_id, {"hp", 100 ... }, nil)

    --[[
    rpc(eid.distributed.mysql_insert, module_id, {"hp", 100 ...}, function(res, progress, succ)
        
    end)
    ]]--

    -- 更新到数据库 
end


return entity