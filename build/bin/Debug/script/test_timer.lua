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
	
	log_m_ = dispatch(eid.app_id, eid.get_module, {"LogModule"})[1]
	timer_m_ = dispatch(eid.app_id, eid.get_module, {"TimerModule"})[1]

end

module.init = function()
	print("init")
	
	listen(module_id, eid.timer.timer_arrive, onTimer)

	millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, {module_id, 20})[1]
	print(millisecond_timer_id)
end

module.execute = function()
end

module.shut = function()
end

----------

function onTimer(args)
	timer_id = args[1]

	if timer_id == millisecond_timer_id then
		print("on timer")

		millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, {module_id, 20})[1]
	end

	return {}
end