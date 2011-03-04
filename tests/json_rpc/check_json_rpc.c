#include "check_json_rpc.h"

#include "check_json_rpc_process.h"

Suite *make_json_rpc_suite (void)
{
	Suite *s = suite_create ("json_rpc");

	/* Json_parser parse test case */
	suite_add_tcase (s, json_rpc_process_tcase());

	return s;
}