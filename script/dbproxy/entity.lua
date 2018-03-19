
local entity = {

    -- db property
    property = {
        id = 0,
        name = "''",
        hp = 100,
        lv = 1,
        loginTime = 0,
    },

    -- 用于标记需要更新到数据库的字段
    dirty = {

    },

    create_sql = function() end,
    init = function(args) end,
    update = function() end,    

    setProperty = function(type, val) end,
}

entity.create_sql = function(self)
    _sql = string.format("insert into Entity (id,name,hp,lv,loginTime) values (%d,%s,%d,%d,%d);"
        , self.property.id
        , self.property.name
        , self.property.hp
        , self.property.lv
        , os.time())
    
    return _sql
end

entity.init = function(self, args)
    dump(args)
    -- 从数据库请求数据
    self.property.id = args[1]
    self.property.name = args[2]
    self.property.hp = args[3]
    self.property.lv = args[4]
    self.property.loginTime = args[5]

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

    it = {}

    for k, v in pairs(self.dirty) do
        table.insert(it, k)
        table.insert(it, v)
    end

    self.dirty = {}

    rpc(eid.distributed.mysql_update, module_id, evpack:entity_update("Entity", self.property.id, it), nil)

    -- 更新到数据库 
end


return entity