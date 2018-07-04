module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

acceptor_m_ = 0
port_ = 0

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

	self:logInfo("[M]Room [F]before_init moduleID : " .. module_id)
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

    listen(eid.test.create_dynamic_acceptor, function(args)
        port_ = args[1]
        --self:logWarn("make acceptor port : " .. tostring(_port))

        acceptor_m_ = APP:createDynamicModule("TcpAcceptorModule")
        self:logInfo("create dynamic module id : " .. tostring(acceptor_m_))
    end)

    self:delay(1000, function() 
        self:logInfo("acceptor init succ m : " .. tostring(acceptor_m_) .. ", p : " .. tostring(port_))
        dispatch(acceptor_m_, eid.network.tcp_make_acceptor, "127.0.0.1", port_)
	end)
	
	self:delay(10000, function() 
		dispatch(APP:getModule("RoomMgrModule"), eid.test.delete_room_module, module_id)
	end)
    
    dispatch(APP:getModule("RoomMgrModule"), eid.test.dynamic_module_init_succ, module_id)

end

module.execute = function()

end

module.shut = function()

	self:logInfo("delete [M]acceptor " .. tostring(acceptor_m_))
	APP:deleteModule(acceptor_m_)

	self:logInfo("[M]room " .. tostring(module_id) .. " shut")
end

module.after_shut = function()
	self:logInfo("[M]room " .. tostring(module_id) .. "after_shut")
end