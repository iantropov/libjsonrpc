#include "check_json_parser_parse.h"

START_TEST (test_valid)
{
	char *s = "2";
    struct json_object *j_int = json_parser_parse(s);

    fail_unless(json_type(j_int) == json_type_int, "bad type");
    fail_unless(json_int_get(j_int) == 2, "bad value");

    json_ref_put(j_int);
}
END_TEST

START_TEST (test_valid_2)
{
	char *s = "[2  ,  3]";
    struct json_object *obj = json_parser_parse(s);

    fail_unless(json_type(obj) == json_type_array, "bad type");
    fail_unless(json_array_length(obj) == 2, "bad length");

    struct json_object *j_int0 = json_array_get(obj, 0);
    struct json_object *j_int1 = json_array_get(obj, 1);
    fail_unless(json_type(j_int0) == json_type_int, "bad type");
    fail_unless(json_type(j_int1) == json_type_int, "bad type");

    fail_unless(json_int_get(j_int0) == 2, "bad value");
    fail_unless(json_int_get(j_int1) == 3, "bad value");

    json_ref_put(obj);
}
END_TEST

START_TEST (test_valid_3)
{
	char *s = "[{\"key1\":2  ,  \"key2\":\"val\"}, 45, null]";
    struct json_object *j_ar = json_parser_parse(s);

    fail_unless(json_type(j_ar) == json_type_array, "bad type");
    fail_unless(json_array_length(j_ar) == 3, "bad length");

    struct json_object *j_obj = json_array_get(j_ar, 0);
    struct json_object *j_int1 = json_array_get(j_ar, 1);
    struct json_object *j_null = json_array_get(j_ar, 2);

    fail_unless(json_type(j_obj) == json_type_object, "bad type");
    fail_unless(json_type(j_int1) == json_type_int, "bad type");
    fail_unless(json_type(j_null) == json_type_null, "bad type");

    struct json_object *j_int2 = json_object_get(j_obj, "key1");
    struct json_object *j_str = json_object_get(j_obj, "key2");

    fail_unless(json_int_get(j_int1) == 45, "bad value");
    fail_unless(json_int_get(j_int2) == 2, "bad value");
    fail_unless(strcmp(json_string_get(j_str), "val") == 0, "bad value");

    json_ref_put(j_ar);
}
END_TEST

static void check_as_invalid(char *s)
{
	struct json_object *j_int = json_parser_parse(s);
	fail_unless(j_int == NULL, "bad result");
}

START_TEST (test_invalid)
{
	check_as_invalid("2-");
	check_as_invalid("2,");
	check_as_invalid("[2,");
	check_as_invalid(",");
	check_as_invalid("23 23");
	check_as_invalid("23,2");
	check_as_invalid("{2,2");
	check_as_invalid("{2,2}");
	check_as_invalid("{\"cd\":,}");
	check_as_invalid("");
	check_as_invalid("p1=v1&p2=v2");
	check_as_invalid("[2,3][");
}
END_TEST

TCase *json_parser_parse_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create ("json_parser_parse");
	tcase_add_test (tc, test_valid);
	tcase_add_test (tc, test_valid_2);
	tcase_add_test (tc, test_valid_3);

	tcase_add_test (tc, test_invalid);
	
	return tc;
}
