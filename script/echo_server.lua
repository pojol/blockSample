module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

acceptor_m_ = 0


module.before_init = function(dir)

    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

	acceptor_m_ = APP:getModuleID("AcceptorModule")
	self:logInfo("acceptor module " .. acceptor_m_)
end

module.init = function()

	listen(event.tcp_new_connect, function(args)
		self:logInfo("new connect fd : " .. args[1])
	end)

	listen(event.tcp_dis_connect, function(args)
		self:logInfo("dis connect fd : " .. args[1])
	end)

	listen(event.tcp_recv, function(args)
		_fd = args[1]
		_msgid = args[2]

		self:logInfo("recv : " .. args[3])

		dispatch(acceptor_m_, event.tcp_send, _fd, 10002, "block!")
	end)

	-- 创建接收器
	dispatch(acceptor_m_, event.tcp_make_acceptor, "127.0.0.1", 8001)

end