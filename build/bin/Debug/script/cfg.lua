
module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}


-- the id of the proc in the cluster
local distributed_id = 7001

-- acceptor cfg
local acceptor_ip = "127.0.0.1"
local acceptor_port = 7001

-- connector cfg
local connector_list = { {"127.0.0.1", 8001}, }

-- root cfg
local root_ip = "127.0.0.1"
local root_port = 10001

-- modules cfg, regist module(container) 2 coordinate
-- module_name, module_characteristic (default 0
local modules = { {"GateLoginModule", 0}, }


module.before_init = function(dir)

	print("cfg init", module_id)

	-- 7002 == eid::sample::get_cfg , tmp
	event:llisten(module_id, module_id, 7001, function (args, callback) 
		local _args = Args.new()
		_args:push_i32(distributed_id)
		_args:push_string(acceptor_ip)
		_args:push_i32(acceptor_port)
		
		local _clen = #connector_list
		_args:push_i32(_clen)
		for i = 1, _clen do
			local _cnod = connector_list[i]
			_args:push_string(_cnod[1])
			_args:push_i32(_cnod[2])
		end

		_args:push_string(root_ip)
		_args:push_i32(root_port)
				
		local _mlen = #modules
		_args:push_i32(_mlen)
		for i = 1, _mlen do
			local _mnod = modules[i]
			_args:push_string(_mnod[1])
			_args:push_i32(_mnod[2])
		end

		callback(_args)
	end)

end