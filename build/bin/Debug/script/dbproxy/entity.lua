
local entity = {
    id,
    name,
    hp,
    lv,
    loginTime,

    setProperty = function(type, val) end,
    update = function() end,    

    dirtyList = {}
}


entity.setProperty = function(type, val)

    if type == 'id' then
        id = val
        dirtyList['id'] = val
    end
    
end

entity.update = function()

    
    dirtyList = {}
end

return entity