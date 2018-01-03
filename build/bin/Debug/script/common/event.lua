
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

function dispatch_getModule(target, moduleName)
    local args = Args.new()
    args:push_string(moduleName)

    buf = event:ldispatch(module_id, target, eid.get_module, args:pop_block(0, args:get_pos()))
    local unpack = Args.new(buf, #buf)
    
    return unpack:pop_i32()
end

function dispatch_delayMilliseconds(target, delay)
    local args = Args.new()
    args:push_i32(module_id)
    args:push_i32(delay)

    buf = event:ldispatch(module_id, target, eid.timer.delay_milliseconds, args:pop_block(0, args:get_pos()))
    local unpack = Args.new(buf, #buf)

    return unpack:pop_ui64()
end

function dispatch_createNode(target, nodeID, moduleID, nodeType, acceptIp, acceptPort, rootIp, rootPort, modules)
    local args = Args.new()
    args:push_i32(nodeID)
    args:push_i32(module_id)
    args:push_string(nodeType)
    
    args:push_string(acceptIp)
    args:push_i32(acceptPort)

    args:push_string(rootIp)
    args:push_i32(rootPort)

    args:push_i32(#modules)
    for i = 1, #modules do
        local mNode = modules[i]
        args:push_string(mNode[1])
        args:push_i32(mNode[2])
        args:push_i32(mNode[3])
    end

    dispatch(target, eid.distributed.node_create, args:pop_block(0, args:get_pos()))
end