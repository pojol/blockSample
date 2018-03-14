module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}


-- the id of the proc in the cluster
local node_id = 7001
local nodeType = "gameNode"

-- acceptor cfg
local acceptor_ip = "127.0.0.1"
local acceptor_port = 7001

-- root cfg
local root_ip = "127.0.0.1"
local root_port = 10001

-- modules cfg, regist module(container) 2 coordinate
-- module_name, module_characteristic (default 0
local modules = { {"GameModule", 0, 0}, }
---------------------------

local log_m_
local node_m_

local game_m_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	log_m_ = APP:get_module("LogModule")
	print("log : " .. log_m_)

	node_m_ = APP:get_module("NodeModule")
	print("node : " .. node_m_)

	game_m_ = APP:get_module("GameModule")
	print("game : " .. game_m_)

	-- init modules id
	for i = 1, #modules do
		local _name = modules[i][1]
		local _moduleid = APP:get_module(_name) 
		print("game modules id : " .. _moduleid)

		modules[i][2] = _moduleid
	end
end

module.init = function()

	listen(module_id, eid.node.node_create_succ, function(args) 
		
		print("game node create success!")
		dispatch(game_m_, eid.sample.create_node_succ, evpack:res_nodinfo(acceptor_ip, acceptor_port, node_id))

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
end

module.execute = function()
end

module.shut = function()
end