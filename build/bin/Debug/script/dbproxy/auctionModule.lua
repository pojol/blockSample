module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

log_m_ = 0
timer_m_ = 0

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
    table.insert(package_path, dir .. "/protobuf/?.lua")
    table.insert(package_path, dir .. "/dbproxy/?.lua")
	package.path = table.concat(package_path, ';')

	require "event"
	require "event_list"

	log_m_ = dispatch_getModule(eid.app_id, "LogModule")
	logInfo("client", "log : " .. log_m_)

    timer_m_ = dispatch_getModule(eid.app_id, "TimerModule")
    logInfo("client", "timer : " .. timer_m_)


end


module.init = function()
    

end

module.execute = function()
end

module.shut = function()
end