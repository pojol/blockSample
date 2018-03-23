module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
timerM_ = 0

-- timer 标记
delayTag = 0

tick_ = 0


module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	logM_ = APP:getModule("LogModule")
	DEBUG_LOG("timer", "log module id", logM_)

	timerM_ = APP:getModule("TimerModule")
	DEBUG_LOG("timer", "timer module id", timerM_)
end

module.init = function()

	listen(eid.timer.timer_arrive, function(args)
	
		if delayTag == args[1] then
			DEBUG_LOG("timer", "arrive", "tag:" .. tostring(delayTag))
			
			dispatch(timerM_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(delayTag, 1000))
		end
		
	end)

	dispatch(timerM_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(delayTag, 1000))
end

module.execute = function()
end

module.shut = function()
end
