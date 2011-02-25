#include <string.h>

#include "check_json_object.h"

#define CHECK_STR "Hello"
#define CHECK_INT 5

START_TEST(test_object_new)
{
	struct json_object *obj = json_object_new();

	fail_unless(json_get_type(obj) == json_type_object, "Json object has bad type!");

	json_ref_put(obj);
}
END_TEST

START_TEST(test_object_get)
{
	struct json_object *obj = json_object_new();
	struct json_object *j_int = json_int_new(CHECK_INT);

	fail_unless(json_object_add(obj, "Key1", j_int) == 0, "add error");

	fail_unless(json_object_get(j_int, "Kol") == NULL, "Json object get from non-object elem");

	fail_unless(json_object_get(obj, "Kol") == NULL, "Json object get non-exist elem");

	fail_unless(json_object_get(obj, NULL) == NULL, "Json object get by NULL key");

	fail_unless(json_object_get(obj, "Key1") == j_int, "Json object get error");

	fail_unless(json_int_get(json_object_get(obj, "Key1")) == CHECK_INT, "int get error");

	json_object_del(obj, "Key1");

	fail_unless(json_object_get(obj, "Key1") == NULL, "Json object get error");

	json_ref_put(obj);
}
END_TEST


START_TEST(test_object_add)
{
	struct json_object *obj = json_object_new();
	struct json_object *j_int = json_int_new(CHECK_INT);

	fail_unless(json_object_add(j_int, "KOL", NULL) == -1, "add error");
	fail_unless(json_object_add(obj, NULL, j_int) == -1, "add error");
	fail_unless(json_object_add(obj, "KOL", NULL) == -1, "add error");
	fail_unless(json_object_add(obj, "Key1", j_int) == 0, "add error");

	struct json_object *j_int2 = json_int_new(7);
	fail_unless(json_object_add(obj, "Key1", j_int2) == 0, "add error");
	fail_unless(json_int_get(json_object_get(obj, "Key1")) == 7, "add duplicate value for same key");

	json_ref_put(obj);
}
END_TEST

START_TEST(test_object_del)
{
	struct json_object *obj = json_object_new();
	struct json_object *j_int = json_int_new(CHECK_INT);

	fail_unless(json_object_add(obj, "Key1", j_int) == 0, "add error");

	json_object_del(j_int, "Key1");
	fail_unless(json_int_get(json_object_get(obj, "Key1")) == CHECK_INT, "error after invalid del");

	json_object_del(obj, "Key2");
	fail_unless(json_int_get(json_object_get(obj, "Key1")) == CHECK_INT, "error after invalid del");

	json_object_del(obj, "Key1");
	fail_unless(json_object_get(obj, "Key1") == NULL, "get after del error");

	json_ref_put(obj);
}
END_TEST

START_TEST(test_object_use)
{
	struct json_object *obj_0 = json_object_new();
	struct json_object *ar_0 = json_array_new();
	struct json_object *obj_1 = json_object_new();

	struct json_object *j_int = json_int_new(CHECK_INT);
	struct json_object *j_str = json_string_new(CHECK_STR);
	struct json_object *j_bool = json_boolean_new(TRUE);

	json_object_add(obj_1, "ar_0", ar_0);
	json_object_add(obj_0, "obj_1", obj_1);
	json_object_add(obj_0, "j_bool", j_bool);
	json_object_add(obj_1, "j_int", j_int);
	json_array_add(ar_0, json_ref_get(j_str));

	fail_unless(json_array_length(ar_0) == 1, "bad length");

	fail_unless(strcmp(json_string_get(json_array_get(json_object_get(json_object_get(obj_0,
				"obj_1"), "ar_0"), 0)), CHECK_STR) == 0, "bad str in obj");

	fail_unless(json_int_get(json_object_get(json_object_get(obj_0,
					"obj_1"), "j_int")) == CHECK_INT, "bad int in obj");

	fail_unless(json_boolean_get(json_object_get(obj_0, "j_bool")) == TRUE, "bad bool in obj");

	json_ref_put(obj_0);

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


TCase *json_object_tcase (void)
{
	TCase *tc = tcase_create("json_object");

	tcase_add_test(tc, test_object_new);
	tcase_add_test(tc, test_object_add);
	tcase_add_test(tc, test_object_del);
	tcase_add_test(tc, test_object_get);
	tcase_add_test(tc, test_object_use);

	return tc;
}
