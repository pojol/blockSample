
local entity = {
    id,
    name,
    hp,
    lv,
    loginTime,

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

    getCreateSql = function() end,

    dirtyList = {}
}

entity.init = function(stream)

    -- 从数据库请求数据

end

entity.setProperty = function(type, val)

    if type == 'id' then
        id = val
        dirtyList['id'] = val
    end
    
end


entity.update = function(self, key, val)

    print(key, val)

    sql = "update Entity set "
    sql = string.format("update Entity set %s=%d where id='%d';", key, val, 110)
    local pack = Args.new()
    pack:push_i32(module_id)
    pack:push_string(sql)
    print(sql)

    rpc(eid.distributed.mysql_query, module_id, pack:pop_block(0, pack:get_pos()), nil)

    -- 更新到数据库
    
end


return entity