#include "check_json_rpc.h"

#include "check_json_rpc_process.h"
#include "check_json_rpc_over_http.h"
#include "check_json_rpc_over_bufevent.h"
#include "check_json_rpc_over_web_sockets.h"

Suite *make_json_rpc_suite (void)
{
	Suite *s = suite_create ("json_rpc");

	/* Json_parser parse test case */
	suite_add_tcase(s, json_rpc_process_tcase());
	suite_add_tcase(s, json_rpc_over_http_tcase());
	suite_add_tcase(s, json_rpc_over_bufevent_tcase());
	suite_add_tcase(s, json_rpc_over_web_sockets_tcase());

	return s;
}
