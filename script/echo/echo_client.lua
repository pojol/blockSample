module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

conn_m_ = 0
log_m_ = 0

module.before_init = function(dir)

    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
    package.path = table.concat(package_path, ';')

    require "utils"

    log_m_ = APP:getModule("LogModule")
	INFO_LOG("client", "log : ", log_m_)

    conn_m_ = APP:getModule("ConnectorModule")
    INFO_LOG("client", "connector module : ", conn_m_)

    INFO_LOG("client", "module id : ", module_id)
end

module.init = function()

    listen(module_id, eid.network.new_connect, function(args)
        INFO_LOG("client", "new connect fd : ", args[1])

        dispatch(conn_m_, eid.network.send, evpack:send2(1001, "hello"))
    end)

    listen(module_id, eid.network.recv, function(args)
        _fd = args[1]
		_msgid = args[2]

        INFO_LOG("client", "recv : " .. args[3])

        --dispatch(conn_m_, eid.network.send, evpack:send2(1001, "hello"))

    end)

    -- 创建连接器
    dispatch(conn_m_, eid.network.make_connector, evpack:make_connector(module_id, "127.0.0.1", 8001))

end