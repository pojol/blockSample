module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

acceptor_m_ = 0
logM_ = 0

module.before_init = function(dir)

    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

	logM_ = APP:getModule("LogModule")
	INFO_LOG("server", "log : ", logM_)

	acceptor_m_ = APP:getModule("AcceptorModule")
	INFO_LOG("server", "acceptor module : ", acceptor_m_)

	INFO_LOG("server", "module id : ", module_id)
end

module.init = function()

	listen(module_id, eid.network.new_connect, function(args)
		INFO_LOG("server", "new connect fd : ", args[1])
	end)

	listen(module_id, eid.network.dis_connect, function(args)
		INFO_LOG("server", "dis connect fd : ", args[1])
	end)

	listen(module_id, eid.network.recv, function(args)
		_fd = args[1]
		_msgid = args[2]

		INFO_LOG("server", "recv : ", args[3])

		dispatch(acceptor_m_, eid.network.send, evpack:send(_fd, 1002, "gsf!"))
	end)

	-- 创建接收器
	dispatch(acceptor_m_, eid.network.make_acceptor, evpack:make_acceptor(module_id, "127.0.0.1", 8001))

end