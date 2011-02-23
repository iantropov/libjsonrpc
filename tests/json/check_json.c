#include <stdlib.h>
#include <check.h>
#include "../../src/json/json.h"


START_TEST (test_new_int)
{
    struct json_object *json_int = json_new_int(5);
    fail_unless(json_get_type(json_int) == json_type_int,
		"json_object has bad type!");
    fail_unless(json_get_int(json_int) == 5,
		"json_object has bad value!");
}
END_TEST

Suite *
json_suite (void)
{
  Suite *s = suite_create ("json");

  /* Core test case */
  TCase *tc_core = tcase_create ("json_new");
  tcase_add_test (tc_core, test_new_int);
  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
  int number_failed;
  Suite *s = json_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
