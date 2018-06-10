
event = {
	
	module_init_succ = 101,
	module_shut_succ = 102,

	rpc_begin = 1001,
	coordinat_adjust_weight = 1002,
	rpc_end = 1999,

	tcp_make_acceptor = 2001,
	tcp_make_connector = 2002,
	tcp_kick_connect = 2003,
	tcp_send = 2004,
	tcp_recv = 2005,
	tcp_new_connect = 2006,
	tcp_dis_connect = 2007,

	script_reload = 2301,

	db_connect = 2401,
	db_execSql = 2402,
	db_insert = 2403,
	db_load = 2404,
	db_update = 2405,
	db_callback = 2406,

	node_init = 2501,
}