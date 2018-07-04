module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

conn_m_ = 0

module.before_init = function(dir)
    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

    conn_m_ = APP:getModuleID("ConnectorModule")
    self:logDebug("[M]echo_client connector module : " .. conn_m_)
end

module.init = function()

    listen(event.tcp_new_connect, function(args)
        self:logInfo("[BEGIN] [M]echo_client [Ev]new_connect")
        self:logInfo("[In] fd:" .. args[1])

        dispatch(conn_m_, event.tcp_send, args[1], 10001, "hello")

        self:logInfo("[END] [M]echo_client [Ev]new_connect")
    end)

    listen(event.tcp_recv, function(args)
        _fd = args[1]
		_msgid = args[2]

        self:logInfo("recv : " .. args[3])

        dispatch(conn_m_, event.tcp_send, _fd, 10001, "hello")
    end)

    -- 创建连接器
    dispatch(conn_m_, event.tcp_make_connector, "127.0.0.1", 8001)
end