#include "check_json_parser.h"

#include "check_json_parser_parse.h"

Suite *make_json_parser_suite (void)
{
	Suite *s = suite_create ("json_parser");

	/* Json_parser parse test case */
	suite_add_tcase (s, json_parser_parse_tcase());

	return s;
}