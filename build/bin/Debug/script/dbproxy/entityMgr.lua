module = {
    before_init = function(dir) end,
	init = function() end,
	execute = function() end,
	shut = function() end,
}

create_sql = string.format("create table if not exists Entity(%s%s%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "mp INT NOT NULL,"
	, "lv INT NOT NULL,"
	, "gold INT NOT NULL,"
	, "loginTime INT NOT NULL")

entity_map = {}

log_m_ = 0
timer_m_ = 0
client_m_ = 0

update_timer_ = 0

function timeArrive(buf, len)
    args = Args.new(buf, len)
    _tid = args:pop_ui64()

    if _tid == update_timer_ then

        for key, val in pairs(entity_map) do
            entity_map[key].update()
        end

    end
end

module.before_init = function(dir)
	local package_path = {}
	table.insert(package_path, dir .. "/common/?.lua")
	table.insert(package_path, dir .. "/protobuf/?.lua")
	package.path = table.concat(package_path, ';')

	require "event"
	require "event_list"

	log_m_ = dispatch_getModule(eid.app_id, "LogModule")
	logInfo("entity", "log : " .. log_m_)

    timer_m_ = dispatch_getModule(eid.app_id, "TimerModule")
    logInfo("entity", "timer : " .. timer_m_)

    client_m_ = dispatch_getModule(eid.app_id, "DBClientModule")
    logInfo("entity", "client : " .. client_m_)

end

module.init = function()
end

module.execute = function()
end

module.shut = function()
end