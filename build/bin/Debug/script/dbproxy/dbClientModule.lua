module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

log_m_ = 0
timer_m_ = 0
node_m_ = 0

millisecond_timer_id = 0

configNode = nil
entityMgr = nil

function init_entitymgr()

    entityMgr = require "entityMgr"
    entityMgr:init()

end

function onTimer(buf, len)
	args = Args.new(buf, len)
	timer_id = args:pop_ui64()

	if timer_id == millisecond_timer_id then
        if entityMgr ~= nil and entityMgr.state_ == EntityState.usable then

            print("start server!")
            entityMgr:entityLoad(0)

        else
            millisecond_timer_id = dispatch_delayMilliseconds(timer_m_,  200)
        end
	end

	return ""
end

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

    configNode = require "configNode"
    configNode:init()

end

module.init = function()

    listen(module_id, eid.node.node_create_succ, function(args)
    
        logInfo("client", "create node succ!")

        local rpcArgs = Args.new()
        rpcArgs:push_string("DBProxyServerModule")
        rpcArgs:push_i32(0)
        rpc(eid.distributed.coordinat_select, module_id, rpcArgs:pop_block(0, rpcArgs:get_pos()), function(buf, len, progress, cbResult)
        
            resArgs = Args.new(buf, len)

            if cbResult == true then
                _nodid = resArgs:pop_i32()
                _nodType = resArgs:pop_string()
                _nodWeight = resArgs:pop_ui32()
                _acceptorIP = resArgs:pop_string()
                _acceptorPort = resArgs:pop_i32()

                -- 后面改成自动注册
                dispatch_registNode(node_m_, module_id, eid.distributed.mysql_query, _acceptorIP, _acceptorPort)
                dispatch_registNode(node_m_, module_id, eid.distributed.mysql_execute, _acceptorIP, _acceptorPort)
            else
                logWarn("client", resArgs:pop_string())
            end

        end)

        return ""
    end)

    listen(module_id, eid.node.node_regist_succ, function(args) 
    
        print("node regist succ")
        init_entitymgr()

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

    listen(module_id, eid.timer.timer_arrive, onTimer)
    millisecond_timer_id = dispatch_delayMilliseconds(timer_m_, 200)

end

module.execute = function()
end

module.shut = function()
end