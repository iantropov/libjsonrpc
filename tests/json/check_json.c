#include <stdlib.h>
#include <check.h>
#include "../../src/koko/koko.h"


START_TEST (test_give_five)
{
  fail_unless (give_five() == 5,
	       "You don`t give me five!");
}
END_TEST

Suite *
koko_suite (void)
{
  Suite *s = suite_create ("Koko");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_give_five);
  suite_add_tcase (s, tc_core);

  return s;
}

int
main (void)
{
  int number_failed;
  Suite *s = koko_suite ();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
