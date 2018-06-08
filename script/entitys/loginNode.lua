module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

node_m_ = 0
node_cfg = {
	node_id = 8001,
    node_type = "LoginNode",
    node_ip = "127.0.0.1",
    node_port = 8001,
    
    -- 必须拥有一个root服务器
    root_ip = "127.0.0.1",
    root_port = 10001,

	-- app 中需要被注册进root的module列表 (服务列表
	modules = { {"LoginModule", module_id} }
}

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"
	
	node_m_ = APP:getModuleID("NodeModule")
	self:logInfo("[M]login node module : " .. node_m_)


	dispatch(node_m_, event.node_init, node_cfg)
end


module.init = function()

end

module.execute = function()

end

module.shut = function()
end