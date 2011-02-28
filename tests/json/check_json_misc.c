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


TCase *json_misc_tcase (void)
{
	TCase *tc = tcase_create("json_misc");

	tcase_add_test(tc, test_ref_null_obj);
	tcase_add_test(tc, test_ref_use);

	return tc;
}
