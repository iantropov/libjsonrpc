#include "check_json_parser_parse.h"

START_TEST (test_init)
{
    struct json_parser *jp = json_parser_init();
    fail_unless(jp != NULL, "init failed");
}
END_TEST

TCase *json_parser_parse_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create ("json_parser_parse");
	tcase_add_test (tc, test_init);
	
	return tc;
}
