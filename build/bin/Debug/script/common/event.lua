
function dump ( t )  
    local print_r_cache={}
    local function sub_print_r(t,indent)
        if (print_r_cache[tostring(t)]) then
            print(indent.."*"..tostring(t))
        else
            print_r_cache[tostring(t)]=true
            if (type(t)=="table") then
                for pos,val in pairs(t) do
                    if (type(val)=="table") then
                        print(indent.."["..pos.."] => "..tostring(t).." {")
                        sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                        print(indent..string.rep(" ",string.len(pos)+6).."}")
                    elseif (type(val)=="string") then
                        print(indent.."["..pos..'] => "'..val..'"')
                    else
                        print(indent.."["..pos.."] => "..tostring(val))
                    end
                end
            else
                print(indent..tostring(t))
            end
        end
    end
    if (type(t)=="table") then
        print(tostring(t).." {")
        sub_print_r(t,"  ")
        print("}")
    else
        sub_print_r(t,"  ")
    end
    print()
end

function dispatch(target, eventID, args, func)
    return event:ldispatch(module_id, target, eventID, args, func)
end

function listen(target, eventID, func)
	event:llisten(module_id, target, eventID, func)
end

function rpc(eventID, moduleid, args, callback)
    event:lrpc(module_id, eventID, moduleid, args, callback)
end

function dispatch_getModule(target, moduleName)
    local args = Args.new()
    args:push_string(moduleName)

    buf = event:ldispatch(module_id, target, eid.get_module, args:pop_block(0, args:get_pos()))
    local unpack = Args.new(buf, #buf)
    
    return unpack:pop_i32()
end

function logInfo(moduleName, info)
    local args = Args.new()
    args:push_ui16(2)
    args:push_string(moduleName)
    args:push_string(info)
    dispatch(log_m_, eid.log.print, args:pop_block(0, args:get_pos()))
end

function logWarn(moduleName, warn)
    local args = Args.new()
    args:push_ui16(1)
    args:push_string(moduleName)
    args:push_string(warn)
    dispatch(log_m_, eid.log.print, args:pop_block(0, args:get_pos()))
end

function logError(moduleName, err)
    local args = Args.new()
    args:push_ui16(0)
    args:push_string(moduleName)
    args:push_string(err)
    dispatch(log_m_, eid.log.print, args:pop_block(0, args:get_pos()))
end

function dispatch_delayMilliseconds(target, delay)

    local args = Args.new()
    args:push_i32(module_id)
    args:push_i32(delay)

    buf = event:ldispatch(module_id, target, eid.timer.delay_milliseconds, args:pop_block(0, args:get_pos()))
    local unpack = Args.new(buf, #buf)

    return unpack:pop_ui64()
end

function dispatch_CreateConnctor(target, ip, port)

    local args = Args.new()
    args:push_i32(module_id)
    args:push_string(ip)
    args:push_i32(port)
    dispatch(target, eid.network.make_connector, args:pop_block(0, args:get_pos()))

end

function dispatch_registNode(target, base, event, ip, port)

    local pack = Args.new()
    pack:push_i32(base)
    pack:push_i32(event)
    pack:push_string(ip)
    pack:push_i32(port)

    dispatch(target, eid.node.node_regist, pack:pop_block(0, pack:get_pos()))
end

function dispatch_createNode(target, nodeID, moduleID, nodeType, acceptor_ip, acceptor_port, rootIp, rootPort, modules)
    local args = Args.new()
    args:push_i32(nodeID)
    args:push_i32(module_id)
    args:push_string(nodeType)

    args:push_string(acceptor_ip)
    args:push_i32(acceptor_port)

    args:push_string(rootIp)
    args:push_i32(rootPort)

    args:push_i32(#modules)
    for i = 1, #modules do
        local mNode = modules[i]
        args:push_string(mNode[1])
        args:push_i32(mNode[2])
        args:push_i32(mNode[3])
    end

    dispatch(target, eid.node.node_create, args:pop_block(0, args:get_pos()))
end