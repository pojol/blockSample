module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}

logM_ = 0

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	package.path = table.concat(package_path, ';')

	--require "utils"

end

module.init = function()
	self:logInfo("hello, block!")
end

module.execute = function()
end

module.shut = function()
end

module.after_shut = function()
end