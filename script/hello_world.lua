module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0
total_ = 0

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	require "utils"

	listen(10001, function(args)
	
		total_ = total_ + 1
		self:logInfo("[M]hw [Ev]10001 [In] " .. dumpStr(args))

	end)

end

module.init = function()
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end