
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

    require "event"
    require "event_list"
	
	dispatch(eid.app_id, 3, "LogModule", function(args)
		log_m_ = args:pop_i32()
    	print("log " .. log_m_)
	end)

	dispatch(eid.app_id, 3, "TimerModule", function(args)
		timer_m_ = args:pop_i32()
		print("timer " .. timer_m_)
	end)
end

module.init = function()
	print("init")

	listen(module_id, 4005, onTimer)

	dispatch(timer_m_, 4000, module_id, 1000, function(args)
		millisecond_timer_id = args:pop_ui64()
		print("create timer id = " .. millisecond_timer_id)
	end)
end

module.execute = function()
end

module.shut = function()
end


----------


function onTimer(args, callback)
	timer_id = args:pop_ui64()
	if timer_id == millisecond_timer_id then
		print("on timer")
	end

end