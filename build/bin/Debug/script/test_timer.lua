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

	require "event"
	require "event_list"
	
	profiler = require "profiler"
	profiler:start()

	log_m_ = dispatch_getModule(eid.app_id, "LogModule")
	timer_m_ = dispatch_getModule(eid.app_id, "TimerModule")
end

module.init = function()
	print("init")
	
	listen(module_id, eid.timer.timer_arrive, onTimer)

	millisecond_timer_id = dispatch_delayMilliseconds(timer_m_, 20)
end

module.execute = function()
end

module.shut = function()
end

----------

function onTimer(buf, len)
	args = Args.new(buf, len)
	timer_id = args:pop_ui64()

	if timer_id == millisecond_timer_id then
		print("on timer " .. tick_)

		tick_ = tick_ + 1

		if tick_ == 10000 then
			profiler:stop()
		else
			millisecond_timer_id = dispatch_delayMilliseconds(timer_m_,  20)
		end		
	end

	return ""
end