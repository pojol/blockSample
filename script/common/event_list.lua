
eid = {
	
	distributed = {
		rpc_begin = 1001,
		
		coordinat_regist = 1002,
		coordinat_unregit = 1003,
		coordinat_adjust_weight = 1004,
		coordinat_select = 1005,

		mysql_query = 1006,
		mysql_update = 1007,

		rpc_end = 2000,
	},
	
	network = {
		make_acceptor = 2001,
		make_connector = 2002,
		kick_connect = 2003,
		send = 2004,
		recv = 2005,
		new_connect = 2006,
		dis_connect = 2007,
		fail_connect = 2008
	},

	dbProxy = {
		connect = 2401,
		query = 2402,
		insert = 2403,
		select = 2404,		
		update = 2405,
		callback = 2406
	},

	log = {
		print = 2101,
	},

	timer = {
		delay_milliseconds = 2201,
		delay_day = 2202,
		delay_week = 2203,
		delay_month = 2204,
		remove_timer = 2205,
		timer_arrive = 2206,
	},

	node = {
		node_create = 2500,
		node_create_succ = 2501,
		node_regist = 2502,
		node_regist_succ = 2503,
	},

	entity_sample = {
		create_node_succ = 10001,
	}
}