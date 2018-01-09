
create_sql = string.format()

local entity = {
    id,
    name,
    hp,
    lv,
    loginTime,

    init = function(stream) end,
    setProperty = function(type, val) end,
    update = function() end,    

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