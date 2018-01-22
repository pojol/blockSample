local configNode = {
    -- the id of the proc in the cluster
    node_id = 7001,
    nodeType = "dbproxyClientNode",
    root_ip = "127.0.0.1",
    root_port = 10001,
    modules = { },

    init = function() end,
}


configNode.init = function()

    for i = 1, #configNode.modules do
        local _name = configNode.modules[i][1]
		local _moduleid = APP:get_module(_name)
        logInfo("clientNode", "dbproxy modules id : " .. _moduleid)

		configNode.modules[i][2] = _moduleid
	end

end

return configNode