#include "check_json_misc.h"

#define CHECK_INT 5

START_TEST(test_ref_use)
{
	struct json_object *j_int = json_int_new(CHECK_INT);

	fail_unless(json_ref_get(j_int) == j_int, "JRef get error");

	fail_unless(json_ref_get(j_int) == j_int, "JRef get error");

	json_ref_put(j_int);
	json_ref_put(j_int);

	fail_unless(json_int_get(j_int) == CHECK_INT, "J int get error after json_ref_put()");

	json_ref_put(j_int);
}
END_TEST

START_TEST(test_ref_null_obj)
{
	fail_unless(json_ref_get(NULL) == NULL, "JRef get error");

	json_ref_put(NULL);
}
END_TEST

START_TEST(test_equils_1)
{
	struct json_object *j_1 = json_parser_parse("[2]");
	struct json_object *j_2 = json_parser_parse("[2]");

	fail_unless(json_equals(j_1, j_2) == 0, "equils");

	json_ref_put(j_1);
	json_ref_put(j_2);
}
END_TEST

START_TEST(test_equils_2)
{
	struct json_object *j_1 = json_parser_parse("[]");
	struct json_object *j_2 = json_parser_parse("[]");

	fail_unless(json_equals(j_1, j_2) == 0, "equils");

	json_ref_put(j_1);
	json_ref_put(j_2);
}
END_TEST

START_TEST(test_equils_3)
{
	struct json_object *j_1 = json_parser_parse("[2, \"kl\"]");
	struct json_object *j_2 = json_parser_parse("[2, \"cd\"]");

	fail_unless(json_equals(j_1, j_2) != 0, "equils");

	json_ref_put(j_1);
	json_ref_put(j_2);
}
END_TEST


START_TEST(test_equils_4)
{
	struct json_object *j_1 = json_parser_parse("{\"cd\":2, \"cdc\":[23, true, null]}");
	struct json_object *j_2 = json_parser_parse("{\"cdc\":[23, true, null], \"cd\":2}");

	fail_unless(json_equals(j_1, j_2) == 0, "equils");

	json_ref_put(j_1);
	json_ref_put(j_2);
}
END_TEST

START_TEST(test_equils_5)
{
	struct json_object *j_1 = json_parser_parse("2");
	struct json_object *j_2 = json_parser_parse("true");

	fail_unless(json_equals(j_1, j_2) != 0, "equils");

	json_ref_put(j_1);
	json_ref_put(j_2);
}
END_TEST



TCase *json_misc_tcase (void)
{
	TCase *tc = tcase_create("json_misc");

	tcase_add_test(tc, test_ref_null_obj);
	tcase_add_test(tc, test_ref_use);

	tcase_add_test(tc, test_equils_1);
	tcase_add_test(tc, test_equils_2);
	tcase_add_test(tc, test_equils_3);
	tcase_add_test(tc, test_equils_4);
	tcase_add_test(tc, test_equils_5);

	return tc;
}
