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

	dispatch(rpcM_, event.rpc_subscription, "AuctionModule")

	listen(event.rpc_auction_getpage_req, function(args) 
		
		target = args[1]
		type = args[2]
		page = args[3]

		dispatch(rpcM_, event.rpc_auction_getpage_res, target, 1, true, ""
		, {"001":{"id":1, "name":"test1", "price":100}
		  ,"002":{"id":2, "name":"test2", "price":101}})
	
	end)

end

module.execute = function()
end

module.shut = function()
end
