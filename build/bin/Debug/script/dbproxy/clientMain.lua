module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

log_m_ = 0
timer_m_ = 0
node_m_ = 0
connector_m_ = 0

configNode = nil

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

    node_m_ = dispatch_getModule(eid.app_id, "NodeModule")
    logInfo("client", "node : " .. node_m_)

    connector_m_ = dispatch_getModule(eid.app_id, "Client2DBConnector")
    logInfo("client", "connector : " .. connector_m_)

    configNode = require "configNode"
    configNode:init()

end

module.init = function()

    listen(module_id, eid.distributed.node_create_succ, function(args)
    
        logInfo("client", "create node succ!")

        local rpcArgs = Args.new()
        rpcArgs:push_string("DBProxyServerModule")
        rpcArgs:push_i32(0)
        rpc(eid.distributed.coordinat_select, rpcArgs:pop_block(0, rpcArgs:get_pos()), function(buf, len, cbResult)
        
            if cbResult == true then
                
                resArgs = Args.new(buf, len)

                resArgs:pop_ui16()  --fd
                resArgs:pop_i32()   --msgid

                _nodid = resArgs:pop_i32()
                _nodType = resArgs:pop_string()
                _nodWeight = resArgs:pop_ui32()
                _acceptorIP = resArgs:pop_string()
                _acceptorPort = resArgs:pop_i32()

                dispatch_CreateConnctor(connector_m_, _acceptorIP, _acceptorPort)

            else
                logError("client", "distributed select node fail!")
            end

        end)

        return ""
    end)

    dispatch_createNode(node_m_
        , configNode.node_id
        , module_id
        , configNode.nodeType
        , ""
        , 0
        , configNode.root_ip
        , configNode.root_port
        , configNode.modules)

    listen(module_id, eid.network.new_connect, function(args)
    
        print("new connect")

    end)

end

module.execute = function()
end

module.shut = function()
end