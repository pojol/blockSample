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
            entityMgr:entity_load(1)

        else
            millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(module_id, 200))[1]
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

	require "utils"

    log_m_ = APP:get_module("LogModule")
	logInfo("client", "log : " .. log_m_)

    timer_m_ = APP:get_module("TimerModule")
    logInfo("client", "timer : " .. timer_m_)

    node_m_ = APP:get_module("NodeModule")
    logInfo("client", "node : " .. node_m_)

    configNode = require "configNode"
    configNode:init()

end

module.init = function()

    listen(module_id, eid.node.node_create_succ, function(args)
    
        logInfo("client", "create node succ!")

        rpc(eid.distributed.coordinat_select, module_id, evpack:coordinat_select("DBProxyServerModule", 0), function(res, progress, succ)
            print("coordinat select succ!")
            if succ == true then
                _nodid = res[1]
                _nodType = res[2]
                _nodWeight = res[3]
                _acceptorIP = res[4]
                _acceptorPort = res[5]

                -- 后面改成自动注册
                dispatch(node_m_, eid.node.node_regist, evpack:node_regist(module_id, eid.distributed.mysql_query, _acceptorIP, _acceptorPort))
                dispatch(node_m_, eid.node.node_regist, evpack:node_regist(module_id, eid.distributed.mysql_select, _acceptorIP, _acceptorPort))
                dispatch(node_m_, eid.node.node_regist, evpack:node_regist(module_id, eid.distributed.mysql_insert, _acceptorIP, _acceptorPort))
                dispatch(node_m_, eid.node.node_regist, evpack:node_regist(module_id, eid.distributed.mysql_update, _acceptorIP, _acceptorPort))
            else
                logWarn("client", res[1])
            end

        end)

        return ""
    end)

    listen(module_id, eid.node.node_regist_succ, function(args) 
    
        print("node regist succ")
        init_entitymgr()

        return ""
    end)


    dispatch(node_m_, eid.node.node_create, evpack:node_create(
          configNode.node_id
        , configNode.nodeType
        , module_id
        , ""
        , 0
        , configNode.root_ip
        , configNode.root_port
        , configNode.modules))

    listen(module_id, eid.timer.timer_arrive, onTimer)
    millisecond_timer_id = dispatch(timer_m_, eid.timer.delay_milliseconds, evpack:delay_milliseconds(module_id, 200))[1]

end

module.execute = function()
end

module.shut = function()
end