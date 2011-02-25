#include "check_json_array.h"
#include "check_json_new.h"
#include "check_json_misc.h"
#include "check_json_object.h"

#include <stdlib.h>

Suite *json_suite (void)
{
	Suite *s = suite_create ("json");

	/* Json object creation test case */
	suite_add_tcase (s, json_new_tcase());
	
	/*Json array test case */
	suite_add_tcase(s, json_array_tcase());
	
	/*Json misc functions (ref_get, ref_put, to_string) */
	suite_add_tcase(s, json_misc_tcase());
	
	/*Json object test case */
	suite_add_tcase(s, json_object_tcase());

	return s;
}

int main (void)
{
  int number_failed;
  SRunner *sr = srunner_create(json_suite());
  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
