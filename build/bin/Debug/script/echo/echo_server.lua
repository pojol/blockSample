module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

acceptor_m_ = 0
log_m_ = 0

module.before_init = function(dir)

    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

	log_m_ = APP:get_module("LogModule")
	logInfo("server", "log : " .. log_m_)

	acceptor_m_ = APP:get_module("AcceptorModule")
	logInfo("server", "acceptor module : " .. acceptor_m_)

	logInfo("server", "module id : " .. module_id)
end


module.init = function()

	listen(module_id, eid.network.new_connect, function(args)
		logInfo("server", "new connect fd : " .. args[1])
	end)

	listen(module_id, eid.network.dis_connect, function(args)
		logInfo("server", "dis connect fd : " .. args[1])
	end)

	listen(module_id, eid.network.recv, function(args)
		_fd = args[1]
		_msgid = args[2]

		logInfo("server", "recv : " .. args[3])

		dispatch(acceptor_m_, eid.network.send, evpack:send(_fd, 1002, "gsf!"))

	end)

	-- 创建接收器
	dispatch(acceptor_m_, eid.network.make_acceptor, evpack:make_acceptor(module_id, "127.0.0.1", 8001))

end