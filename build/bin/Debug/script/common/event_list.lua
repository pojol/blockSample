
eid = {
	app_id = 1,
	get_module = 3,
	new_dynamic_module = 102,
	delete_dynamic_module = 103,
	
	distributed = {
		rpc_begin = 1001,
		
		node_create = 1002,
		node_create_succ = 1003,
		coordinat_regist = 1004,
		coordinat_unregit = 1005,
		coordinat_adjust_weight = 1006,
		coordinat_get = 1007,

		rpc_end = 2000,
	}
	
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

	sample = {
		create_node_succ = 10001,
	}
}