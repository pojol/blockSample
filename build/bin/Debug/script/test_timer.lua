
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

module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

log_m_ = 0
timer_m_ = 0

millisecond_timer_id = 0

module.before_init = function(dir)
	print("before init")
	local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
    package.path = table.concat(package_path, ';')

    --require "event"
    require "event_list"
	
	event:ldispatch(module_id, eid.app_id, 3, {"LogModule"}, function(args) 
    	log_m_ = args[1]
		print("log " .. log_m_)
	end)

	event:ldispatch(module_id, eid.app_id, 3, {"TimerModule"}, function(args)
		timer_m_ = args[1]
		print("timer " .. timer_m_)
	end)

end

module.init = function()
	print("init")
	event:llisten(module_id, module_id, 4005, onTimer)
	
	event:ldispatch(module_id, timer_m_, 4000, {module_id, 20}, function(args) 
		millisecond_timer_id = args[1]
		print("create timer id = " .. millisecond_timer_id)
	end)
	
end

module.execute = function()
end

module.shut = function()
end


----------


function onTimer(args, callback)
	timer_id = args[1]
	if timer_id == millisecond_timer_id then
		print("on timer")

		event:ldispatch(module_id, timer_m_, 4000, {module_id, 20}, function(args)
			millisecond_timer_id = args[1]
		end)
	end

end