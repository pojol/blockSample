module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}


-- the id of the proc in the cluster
local node_id = 7001
local nodeType = "dbproxyClientNode"

-- root cfg
local root_ip = "127.0.0.1"
local root_port = 10001

-- modules cfg, regist module(container) 2 coordinate
-- module_name, module_characteristic (default 0
local modules = { }
---------------------------

log_m_ = 0

local node_m_
local client_m_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "event"
	require "event_list"

	log_m_ = dispatch_getModule(eid.app_id, "LogModule")
	logInfo("clientNode", "log : " .. log_m_)

	node_m_ = dispatch_getModule(eid.app_id, "NodeModule")
	logInfo("clientNode", "node : " .. node_m_)

	client_m_ = dispatch_getModule(eid.app_id, "DBClientModule")
	logInfo("clientNode", "client : " .. client_m_)

	-- init modules id
	for i = 1, #modules do
		local _name = modules[i][1]
		local _moduleid = dispatch_getModule(eid.app_id, _name)
		logInfo("clientNode", "dbproxy modules id : " .. _moduleid)

		modules[i][2] = _moduleid
	end
end

module.init = function()

	listen(module_id, eid.distributed.node_create_succ, function(args) 
		logInfo("clientNode", "dbproxy node create success!")

		local pack = Args.new()
		pack:push_i32(node_id)

		dispatch(client_m_, eid.sample.create_node_succ, pack:pop_block(0, pack:get_pos()))

		return ""
	end)

	dispatch_createNode(node_m_, node_id, module_id, nodeType, "", 0 , root_ip, root_port, modules)

end

module.execute = function()
end

module.shut = function()
end