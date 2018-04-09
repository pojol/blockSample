local at_uint8 = 1
local at_int8 = 2
local at_uint16 = 3
local at_int16 = 4
local at_uint32 = 5
local at_int32 = 6
local at_uint64 = 7
local at_int64 = 8
local at_bool = 9
local at_float = 10
local at_double = 11
local at_string = 12
local at_list = 13
local at_vec = 14
local at_map = 15
local at_eof = 20


-- auto generated
evpack = {

    coordinat_select = function(self, module_name, module_fature)
        print(module_name, module_fature)
        local pack = Args.new()
        pack:push_string(module_name)
        pack:push_i32(module_fature)
        return pack:exportBuf()
    end,

    dbQuery = function(self, sql)
        local pack = Args.new()
        pack:push_string(sql)
        return pack:exportBuf()
    end,

    dbLoad = function(self, id)
        local pack = Args.new()
        pack:push_i32(id)
        return pack:exportBuf()
    end,

    dbInsert = function(self, buf)
        local pack = Args.new()
        pack:push_string(buf)
        return pack:exportBuf()
    end,

    dbUpdate = function(self, id, buf)
        local pack = Args.new()
        pack:push_i32(id)
        pack:push_string(buf)
        return pack:exportBuf()
    end,

    mysql_connect = function(self, host, user, password, db, port, useCache)
        local pack = Args.new()
        pack:push_string(host)
        pack:push_string(user)
        pack:push_string(password)
        pack:push_string(db)
        pack:push_i32(port)
        pack:push_bool(useCache)
        return pack:exportBuf()
    end,

    delay_milliseconds = function(self, tag, milliseconds)
        local pack = Args.new()
        pack:push_i32(tag)
        pack:push_i32(milliseconds)
        return pack:exportBuf()
    end,

    node_regist = function(self, module_id, reg_event, ip, port)
        local pack = Args.new()
        pack:push_i32(module_id)
        pack:push_i32(reg_event)
        pack:push_string(ip)
        pack:push_i32(port)
        return pack:exportBuf()
    end,

    node_create = function(self, node_id, node_type, module_id, ip, port, rootIp, rootPort, modules)
        
        local pack = Args.new()
        pack:push_i32(node_id)
        pack:push_i32(module_id)
        pack:push_string(node_type)

        pack:push_string(ip)
        pack:push_i32(port)

        pack:push_string(rootIp)
        pack:push_i32(rootPort)

        pack:push_i32(#modules)
        for i = 1, #modules do
            local mNode = modules[i]
            pack:push_string(mNode[1])
            pack:push_i32(mNode[2])
            pack:push_i32(mNode[3])
        end

        return pack:exportBuf()

    end,

    res_nodinfo = function(self, ip, port, node_id)
        local pack = Args.new()
        pack:push_string(ip)
        pack:push_i32(port)
        pack:push_i32(node_id)
        return pack:exportBuf()
    end,

    make_acceptor = function(self, ip, port)
        local pack = Args.new()
        pack:push_string(ip)
        pack:push_i32(port)
        return pack:exportBuf()
    end,

    make_connector = function(self, ip, port)
        local pack = Args.new()
        pack:push_string(ip)
        pack:push_i32(port)
        return pack:exportBuf()
    end,

    send = function(self, fd, msg_id, buf)
        local pack = Args.new()
        pack:push_ui16(fd)
        pack:push_i32(msg_id)
        pack:push_string(buf)
        return pack:exportBuf()
    end,

    send2 = function(self, msg_id, buf)
        local pack = Args.new()
        pack:push_i32(msg_id)
        pack:push_string(buf)
        return pack:exportBuf()
    end,

    kick_connect = function(self, fd)
        local pack = Args.new()
        pack:push_ui16(fd)
        return pack:exportBuf()
    end
}


function evunpack(buf)
    unpack = Args.new()
    unpack:importBuf(buf)

    _args = {}
    _idx = 1
    _tag = unpack:get_tag()
    while(_tag ~= 0) do

        if _tag == at_uint16 then
            table.insert(_args, _idx, unpack:pop_ui16())
        elseif _tag == at_int32 then
            table.insert(_args, _idx, unpack:pop_i32())
        elseif _tag == at_string then
            table.insert(_args, _idx, unpack:pop_string())
        elseif _tag == at_uint32 then
            table.insert(_args, _idx, unpack:pop_ui32())
        elseif _tag == at_int64 then
            table.insert(_args, _idx, unpack:pop_i64())
        elseif _tag == at_uint64 then
            table.insert(_args, _idx, unpack:pop_ui64())
        elseif _tag == at_bool then
            table.insert(_args, _idx, unpack:pop_bool())
        end
        
        _idx = _idx + 1
        _tag = unpack:get_tag()
    end

    return _args
end