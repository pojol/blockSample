module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

echo_m_ = 0

module.before_init = function(dir)

    local package_path = {}
    table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
    package.path = table.concat(package_path, ';')

    require "event"
    require "event_list"

	echo_m_ = dispatch(eid.app_id, eid.get_module, {"EchoServer"})[1]
	print("echo : " .. echo_m_)
end

module.init = function()

	listen(module_id, 10001, onRecv)
	
end

function onRecv(buf, len)

	local unpack = Args.new(buf, len)
	fd = unpack:pop_ui16()
	msgid = unpack:pop_i32()
	res = unpack:pop_string()
	--print("recv : " .. res)
	

	local pack = Args.new()
	pack:push_ui16(fd)
	pack:push_i32(1002)
	pack:push_string("gsf!")

	dispatch(echo_m_, 10002, pack:pop_block(0, pack:get_pos()))
end

module.execute = function()
	
end


module.shut = function()
end