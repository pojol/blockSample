require "event_list"
require "event_fmt"

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

function deep_copy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for key, value in pairs(object) do
            new_table[_copy(key)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

function dispatch(target, eventID, buf)

    resBuf = event:ldispatch(module_id, target, eventID, buf)
    if #resBuf == 0 then
        return nil
    else
        return evunpack(resBuf, #resBuf)
    end
end

function listen(target, eventID, func)
	event:llisten(module_id, target, eventID, func)
end

function rpc(event_id, module_id, buf, callback)

    function _callback(resBuf, len, progress, succ)
        _args = evunpack(resBuf, len)
        callback(_args, progress, succ)
    end
    
    if callback ~= nil then
        event:lrpc(module_id, event_id, module_id, buf, _callback)
    else
        event:lrpc(module_id, event_id, module_id, buf, nil)
    end
end

function logInfo(moduleName, info)
    local args = Args.new()
    args:push_ui16(2)
    args:push_string(moduleName)
    args:push_string(info)
    event:ldispatch(module_id, log_m_, eid.log.print, args:pop_block(0, args:get_size()), nil)
end

function logWarn(moduleName, warn)
    local args = Args.new()
    args:push_ui16(1)
    args:push_string(moduleName)
    args:push_string(warn)
    event:ldispatch(module_id, log_m_, eid.log.print, args:pop_block(0, args:get_size()), nil)
end

function logError(moduleName, err)
    local args = Args.new()
    args:push_ui16(0)
    args:push_string(moduleName)
    args:push_string(err)
    event:ldispatch(module_id, log_m_, eid.log.print, args:pop_block(0, args:get_size()), nil)
end