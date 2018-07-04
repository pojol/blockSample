module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

rpcM_ = 0

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	rpcM_ = APP:getModule("RPCModule")
	
end

module.init = function()

	-- 获取拍卖行 单页数据
	-- 
	page = 1
	dispatch(rpcM_, event.rpc, "AuctionModule", event.rpc_auction_getpage_req, "type", page)

	listen(event.rpc_auction_getpage_res, function(args) 
		count = args[1]
		succ = args[2]
		errMsg = args[3]

		if succ then
			pageInfo = args[4]
		else
			pageInfo = {}
		end


	end)
end

module.execute = function()
end

module.shut = function()
end
