


function dispatch(...)
	arg = { ... }
	
	local _len = #arg
	local _args = Args.new()
	
    for k,v in pairs(arg) do
		
		if k > 2 then
			if type(v) == "number" then
				_args:push_i32(v)
			end
			if type(v) == "string" then
				_args:push_string(v)
			end
		end

	end

	if not arg[1] then
		print_warning("warning", "target id = nil")
	end
	if not arg[2] then
		print_warning("warning", "event id = nil")
	end
	event:ldispatch(module_id, arg[1], arg[2], _args, arg[_len])
end

function listen(self_id, event_id, func)
	event:llisten(module_id, self_id, event_id, func)
end