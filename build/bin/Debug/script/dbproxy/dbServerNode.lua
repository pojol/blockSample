module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}


-- the id of the proc in the cluster
local node_id = 7002
local nodeType = "dbproxyServerNode"

-- acceptor cfg
local acceptor_ip = "127.0.0.1"
local acceptor_port = 7002

-- root cfg
local root_ip = "127.0.0.1"
local root_port = 10001

-- modules cfg, regist module(container) 2 coordinate
-- module_name, module_id, module_feature (default 0
local modules = { {"DBProxyServerModule", 0, 0}, }
---------------------------

local log_m_
local node_m_

local db_m_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "event"
	require "event_list"

	log_m_ = dispatch(eid.app_id, eid.get_module, {"LogModule"})[1]
	print("log : " .. log_m_)

	node_m_ = dispatch(eid.app_id, eid.get_module, {"NodeModule"})[1]
	print("node : " .. node_m_)

	db_m_ = dispatch(eid.app_id, eid.get_module, {"DBProxyServerModule"})[1]
	print("dbproxy : " .. db_m_)

	-- init modules id
	for i = 1, #modules do
		local _name = modules[i][1]
		local _moduleid = dispatch(eid.app_id, eid.get_module, {"_name"})[1]
		print("dbproxy modules id : " .. _moduleid)

		modules[i][2] = _moduleid
	end
end

module.init = function()

	listen(module_id, eid.node.node_create_succ, function(args) 
		
		print("dbproxy node create success!")

		dispatch(db_m_, eid.sample.create_node_succ, {acceptor_ip, acceptor_port, node_id})

		return ""
	end)

	dispatch_createNode(node_m_
		, node_id
		, module_id
		, nodeType
		, acceptor_ip
		, acceptor_port
		, root_ip
		, root_port
		, modules)

end

module.execute = function()
end

module.shut = function()
end