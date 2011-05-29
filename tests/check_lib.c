#include <check.h>

#include "json/check_json.h"
#include "json_parser/check_json_parser.h"
#include "json_rpc/check_json_rpc.h"

#include <stdlib.h>

int main (void)
{
  int number_failed;
//  SRunner *sr = srunner_create(make_json_suite());
//  SRunner *sr = srunner_create(make_json_parser_suite());
//  srunner_add_suite(sr, make_json_parser_suite());
  SRunner *sr = srunner_create(make_json_rpc_suite());
//  srunner_add_suite(sr, make_json_rpc_suite());

  srunner_run_all (sr, CK_NORMAL);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
