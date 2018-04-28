module = {
	before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
	after_shut = function() end,
}



module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')


end

module.init = function()

	self:delay(5000, function() 
		self:logInfo("timer arrived!")
	end)

end

module.execute = function()
end

module.shut = function()
end
