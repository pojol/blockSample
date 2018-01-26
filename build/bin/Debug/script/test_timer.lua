module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
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
	
	profiler = require "profiler"
	profiler:start()

	log_m_ = APP:get_module("LogModule")
	logInfo("timer", "log : " .. log_m_)

	timer_m_ = APP:get_module("TimerModule")
	logInfo("timer", "timer : " .. timer_m_)
end

module.init = function()
	print("init")

	listen(module_id, eid.timer.timer_arrive, onTimer)

	millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(module_id, 1000))[1]
	logInfo("timer", "timer id : " .. millisecond_timer_id)
end

module.execute = function()
end

module.shut = function()
end

----------

function onTimer(buf, len)
	args = Args.new(buf, len)
	timer_id = args:pop_ui64()

	logInfo("timer", "arrive timer id : " .. timer_id)

	if timer_id == millisecond_timer_id then
		print("on timer " .. tick_)
		--[[
		tick_ = tick_ + 1

		if tick_ == 10000 then
			profiler:stop()
		else
			
		end	
		]]--	
		millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, {module_id, 1000})[1]		
	end

	return ""
end