module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}


module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"
	
end

module.init = function()

    listen(event.module_init_succ, function(args)
        self:logInfo("[M]login [Ev]module_init_succ name:" .. args[1] .. " id:" .. args[2])
    
    end)

end