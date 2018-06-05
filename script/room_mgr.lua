module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

acceptor_m_ = 0
port_idx_ = 8002

local protobuf_

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	protobuf_ = require "protobuf"

	addr = io.open("C:/github/hammer_room/depend/proto/lua/hammer.pb","rb")
	buffer = addr:read "*a"
	addr:close()

	protobuf_.register(buffer)

	self:logInfo("[M]RoomMgr [F]before_init moduleID : " .. module_id)

	acceptor_m_ = APP:getModule("StaticTcpAcceptor")
end

module.init = function()

    listen(eid.network.new_connect, function(args) 
		self:logInfo("[M]RoomMgr [Ev]new_connect [In] " .. dumpStr(args))

	end)

	listen(eid.network.dis_connect, function(args)
		self:logInfo("[M]RoomMgr [Ev]dis_connect [In] " .. dumpStr(args))

	end)

	listen(eid.network.recv, function(args)
		self:logInfo("[M]RoomMgr [Ev]recv [In] fd:" .. tostring(args[1]) .. " msgid:" .. tostring(args[2]))
		
		if args[2] == 5 then
			req = protobuf_.decode("msg.ProtoGameReadyReq", args[3])
			self:logInfo("[In] " .. dumpStr(req)) 
		end
	end)

	_roomID = APP:createDynamicModule("RoomModule")
	self:logInfo("create dynamic module id : " .. tostring(_roomID))

	listen(eid.test.dynamic_module_init_succ, function(args)
		self:logInfo("dynamic module : " .. tostring(args[1]) .. " init succ!")
		
		dispatch(args[1], eid.test.create_dynamic_acceptor, port_idx_)
		port_idx_ = port_idx_ + 1
	end)

	listen(eid.test.delete_room_module, function(args)
		self:logInfo("delete module event")

		APP:deleteModule(args[1])
	end)

	dispatch(acceptor_m_, eid.network.tcp_make_acceptor, "127.0.0.1", 8001)
	
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end