
create_sql = string.format("create table if not exists Entity(%s%s%s%s%s%s%s) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;"
    , "id INT NOT NULL PRIMARY KEY AUTO_INCREMENT,"
	, "name VARCHAR(32) NOT NULL,"
	, "hp INT NOT NULL,"
	, "mp INT NOT NULL,"
	, "lv INT NOT NULL,"
	, "gold INT NOT NULL,"
	, "loginTime INT NOT NULL")

entity_map = {}

local entityMgr = {
	
}


return entityMgr