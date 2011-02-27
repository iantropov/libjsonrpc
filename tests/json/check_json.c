#include "check_json.h"
#include "check_json_array.h"
#include "check_json_misc.h"
#include "check_json_object.h"
#include "check_json_new.h"

#include "check.h"
#include <stdlib.h>

Suite *make_json_suite (void)
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