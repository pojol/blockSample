module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

conn_m_ = 0
logM_ = 0

module.before_init = function(dir)
    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

    self:logInfo("[BEGIN] [M]echo_client [T]before_init")
    self:logInfo("[In] dir:" .. dir)

    conn_m_ = APP:getModule("ConnectorModule")
    self:logDebug("connector module : " .. conn_m_)

    self:logInfo("[END] [M]echo_client [T]before_init")
end

module.init = function()
    self:logInfo("[BEGIN] [M]echo_client [T]init")

    listen(eid.network.new_connect, function(args)
        self:logInfo("[BEGIN] [M]echo_client [Ev]new_connect")
        self:logInfo("[In] fd:" .. args[1])

        dispatch(conn_m_, eid.network.send, evpack:send2(10001, "hello"))

        self:logInfo("[END] [M]echo_client [Ev]new_connect")
    end)

    listen(eid.network.recv, function(args)
        _fd = args[1]
		_msgid = args[2]

        self:logInfo("[BEGIN] [M]echo_client [Ev]recv")
        self:logInfo("[In] fd:" .. _fd .. " msg:" .. _msgid)

        dispatch(conn_m_, eid.network.send, evpack:send2(10001, "hello"))
        self:logInfo("[END] [M]echo_client [Ev]recv")
    end)

    -- 创建连接器
    dispatch(conn_m_, eid.network.tcp_make_connector, evpack:make_connector("127.0.0.1", 8001))

    self:logInfo("[END] [M]echo_client [T]init")
end