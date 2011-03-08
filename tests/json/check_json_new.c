#include "check_json_new.h"

#include <math.h>
#include <stdio.h>

#define DOUBLE_EPSILON 0.01

static int is_doubles_equils(double a, double b)
{
	return (fabs(a - b) < DOUBLE_EPSILON) ? 1 : 0;
}

START_TEST (test_new_int)
{
    struct json_object *json_int = json_int_new(5);
    fail_unless(json_type(json_int) == json_type_int,
		"json_object has bad type!");
    fail_unless(json_int_get(json_int) == 5,
		"json_object has bad value!");

    json_ref_put(json_int);
}
END_TEST

START_TEST (test_new_double)
{
    double val = 3.14;
    struct json_object *json_obj = json_double_new(val);
    fail_unless(json_type(json_obj) == json_type_double,
		"json_object has bad type!");
		
    fail_unless(is_doubles_equils(json_double_get(json_obj), val) == 1,
		"json_object has bad value!");

    json_ref_put(json_obj);
}
END_TEST

START_TEST (test_new_boolean)
{
    struct json_object *json_obj = json_boolean_new(TRUE);
    fail_unless(json_type(json_obj) == json_type_boolean,
		"json_object has bad type!");
    fail_unless(json_boolean_get(json_obj) == TRUE,
		"json_object has bad value!");

    json_ref_put(json_obj);
}
END_TEST

START_TEST (test_new_string)
{
	char *s = "Hello";
    struct json_object *json_obj = json_string_new(s);
    s[0] = 'P';
    fail_unless(json_type(json_obj) == json_type_string,
		"json_object has bad type!");
    fail_unless(strcmp(json_string_get(json_obj), "Hello") == 0,
		"json_object has bad value!");

    json_ref_put(json_obj);
}
END_TEST

START_TEST (test_new_string_len)
{
	char *s = "Hello";
    struct json_object *json_obj = json_string_new_len(s, 2);
    fail_unless(json_type(json_obj) == json_type_string,
		"json_object has bad type!");
    fail_unless(strcmp(json_string_get(json_obj), "He") == 0,
		"json_object has bad value!");

    json_ref_put(json_obj);
}
END_TEST

TCase *json_new_tcase(void)
{
	/* Json object creation test case */
	TCase *tc_json_new = tcase_create ("json_new");
	tcase_add_test (tc_json_new, test_new_int);
	tcase_add_test (tc_json_new, test_new_double);
	tcase_add_test (tc_json_new, test_new_boolean);
	tcase_add_test (tc_json_new, test_new_string);
	tcase_add_test (tc_json_new, test_new_string_len);

	return tc_json_new;
}
