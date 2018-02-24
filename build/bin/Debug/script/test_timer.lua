module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

log_m_ = 0
timer_m_ = 0

millisecond_timer_id = 0

tick_ = 0


module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	log_m_ = APP:get_module("LogModule")
	DEBUG_LOG("timer", "log module id", log_m_)

	timer_m_ = APP:get_module("TimerModule")
	DEBUG_LOG("timer", "timer module id", timer_m_)
end

module.init = function()
	DEBUG_LOG("timer", "init status")

	listen(module_id, eid.timer.timer_arrive, onTimer)

	millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(module_id, 1000))[1]
	DEBUG_LOG("timer", "timer id", millisecond_timer_id)
end

module.execute = function()
end

module.shut = function()
end

----------

function onTimer(args)

	timer_id = args[1]

	if timer_id == millisecond_timer_id then
		DEBUG_LOG("timer", "arrive", timer_id)

		millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(module_id, 1000))[1]		
	end

	return ""
end