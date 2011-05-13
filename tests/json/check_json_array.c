#include <string.h>

#include "check_json_array.h"

#define CHECK_STR "Hello"
#define CHECK_INT 5

START_TEST(test_array_create_empty)
{
	struct json_object *ar = json_array_new();

	fail_unless(json_type(ar) == json_type_array, "Json object has bad type!");

	fail_unless(json_array_length(ar) == 0, "Json array has invalid length");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_create)
{
	struct json_object *ar = json_array_new();
	struct json_object *j_int = json_int_new(CHECK_INT);

	fail_unless(json_array_add(ar, j_int) == 0, "Json_array_add fail");

	fail_unless(json_array_add(ar, NULL) == -1, "JA add NULL object");

	fail_unless(json_array_add(j_int, j_int) == -1, "JA add to non-array object");

	fail_unless(json_array_length(ar) == 1, "Json_array has bad length");

	j_int = json_array_get(ar, 0);

	fail_unless(j_int != NULL, "Json_array_get return NULL");

	fail_unless(json_type(j_int) == json_type_int, "Json_int has bad type");

	fail_unless(json_int_get(j_int) == CHECK_INT, "Json_int has bad value");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_use)
{
	struct json_object *ar_0 = json_array_new();
	struct json_object *ar_1 = json_array_new();
	struct json_object *ar_2 = json_array_new();

	fail_unless(json_array_length(ar_0) == 0, "Json_array has bad length");
	fail_unless(json_array_length(ar_1) == 0, "Json_array has bad length");
	fail_unless(json_array_length(ar_2) == 0, "Json_array has bad length");

	struct json_object *j_int = json_int_new(CHECK_INT);
	struct json_object *j_str = json_string_new(CHECK_STR);
	struct json_object *j_bool = json_boolean_new(TRUE);

	json_array_add(ar_0, ar_1);
	json_array_add(ar_0, j_int);

	fail_unless(json_array_length(ar_0) == 2, "Json_array has bad length");
	fail_unless(json_array_length(ar_1) == 0, "Json_array has bad length");

	json_array_add(ar_2, json_ref_get(j_bool));
	json_array_add(ar_2, json_ref_get(j_str));

	fail_unless(json_array_length(ar_2) == 2, "Json_array has bad length");

	json_array_add(ar_1, j_bool);
	json_array_add(ar_1, ar_2);

	fail_unless(json_array_length(ar_0) == 2, "Json_array has bad length");
	fail_unless(json_array_length(ar_0) == 2, "Json_array has bad length");
	fail_unless(json_array_length(ar_0) == 2, "Json_array has bad length");

	struct json_object *j_bool2 = json_array_get(json_array_get(json_array_get(ar_0, 0), 1), 0);
	fail_unless(json_boolean_get(j_bool2) == TRUE, "Json_bool2 has bad value");

	struct json_object *j_str2 = json_array_get(json_array_get(json_array_get(ar_0, 0), 1), 1);
	fail_unless(strcmp(json_string_get(j_str2), CHECK_STR) == 0, "Json_str2 has bad value");

	struct json_object *j_bool3 = json_array_get(json_array_get(ar_0, 0), 0);
	fail_unless(json_boolean_get(j_bool3) == TRUE, "Json_boolean3 has bad value");

	struct json_object *j_int2 = json_array_get(ar_0, 1);
	fail_unless(json_int_get(j_int2) == CHECK_INT, "Json_int2 has bad value");

	json_ref_put(ar_0);

	json_ref_put(j_str);
}
END_TEST

START_TEST(test_array_get)
{
	struct json_object *j_int = json_int_new(5);
	struct json_object *ar = json_array_new();

	fail_unless(json_array_length(ar) == 0, "JA bad length");

	fail_unless(json_array_add(ar, j_int) == 0, "JA add error");

	fail_unless(json_array_length(ar) == 1, "JA bad length");

	fail_unless(json_array_get(ar, 0) == j_int, "JA get at valid index error");

	fail_unless(json_array_get(ar, 1) == NULL, "JA get at invalid index error");

	fail_unless(json_array_get(ar, -11) == NULL, "JA get at invalid index error");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_del_0)
{
	struct json_object *ar = json_array_new();

	json_array_del(ar, 0);

	fail_unless(json_type(ar) == json_type_array, "JA bad type");

	fail_unless(json_array_length(ar) == 0, "JA bad length");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_del_1)
{
	struct json_object *j_int = json_int_new(5);
	struct json_object *ar = json_array_new();

	fail_unless(json_array_add(ar, j_int) == 0, "JA add error");

	json_array_del(ar, 0);

	fail_unless(json_array_length(ar) == 0, "JA bad length");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_del_2)
{
	struct json_object *j_int = json_int_new(5);
	struct json_object *ar = json_array_new();

	fail_unless(json_array_add(ar, j_int) == 0, "JA add error");

	fail_unless(json_array_add(ar, json_ref_get(j_int)) == 0, "JA add error");

	fail_unless(json_array_add(ar, json_ref_get(j_int)) == 0, "JA add error");

	json_array_del(ar, 3);

	fail_unless(json_array_length(ar) == 3, "JA bad length");

	json_array_del(ar, 1);

	fail_unless(json_array_length(ar) == 2, "JA bad length");

	fail_unless(json_array_get(ar, 0) == j_int, "JA get at valid index error");
	fail_unless(json_array_get(ar, 1) == j_int, "JA get at valid index error");

	json_array_del(ar, 1);

	fail_unless(json_array_get(ar, 0) == j_int, "JA get at valid index error");

	fail_unless(json_array_get(ar, 1) == NULL, "JA get at invalid index error");

	json_ref_put(ar);
}
END_TEST

START_TEST(test_array_del_3)
{
	struct json_object *j_int_1 = json_int_new(5);
	struct json_object *j_int_2 = json_int_new(10);
	struct json_object *j_int_3 = json_int_new(15);


	struct json_object *ar = json_array_new();

	fail_unless(json_array_add(ar, j_int_1) == 0, "JA add error");
	fail_unless(json_array_add(ar, j_int_2) == 0, "JA add error");
	fail_unless(json_array_add(ar, j_int_3) == 0, "JA add error");

	json_array_del(ar, 1);

	fail_unless(json_array_length(ar) == 2, "JA bad length");

	fail_unless(json_array_get(ar, 0) == j_int_1, "JA get at valid index error");
	fail_unless(json_array_get(ar, 1) == j_int_3, "JA get at valid index error");

	json_array_del(ar, 1);

	fail_unless(json_array_get(ar, 0) == j_int_1, "JA get at invalid index error");

	json_ref_put(ar);
}
END_TEST

TCase *
json_array_tcase (void)
{
	TCase *tc = tcase_create("json_array");

	tcase_add_test(tc, test_array_create_empty);
	tcase_add_test(tc, test_array_create);

	tcase_add_test(tc, test_array_use);

	tcase_add_test(tc, test_array_get);

	tcase_add_test(tc, test_array_del_0);
	tcase_add_test(tc, test_array_del_1);
	tcase_add_test(tc, test_array_del_2);
	tcase_add_test(tc, test_array_del_3);

	return tc;
}
