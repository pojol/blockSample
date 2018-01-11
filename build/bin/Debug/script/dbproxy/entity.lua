
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


entity.update = function()

    -- 更新到数据库
    
    dirtyList = {}
end


return entity