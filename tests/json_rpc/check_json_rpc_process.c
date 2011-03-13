#include "check_json_rpc_process.h"

#include <event.h>

#include "../../src/json_parser/json_parser.h"
#include "json_rpc_check_util.h"

#define REQUEST_ID "id"
#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"
#define REQUEST_VERSION "jsonrpc"

#define JSON_RPC_VERSION "2.0"

#define ERROR_INVALID_REQUEST_MESSAGE "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND_MESSAGE "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL_MESSAGE "Internal error"
#define ERROR_INTERNAL_CODE -32603

struct json_rpc *jr;
struct json_object *origin, *call;

static void test_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	fail_unless(json_equals(res, *((struct json_object **)arg)) == 0, "invalid result");
	json_ref_put(res);
	event_loopbreak();
}

static void test_notification(struct json_rpc *jr, struct json_object *res, void *arg)
{
	fail_unless(FALSE, "notification");
}

static void setup()
{
	event_init();
	jr = json_rpc_init();
	fail_unless(jr != NULL, "init error");
}

static void teardown()
{
	json_ref_put(call);
	json_ref_put(origin);
	json_rpc_destroy(jr);
}

static void test_1(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_array, "type");
	fail_unless(json_array_length(p) == 1, "length");
	fail_unless(json_type(json_array_get(p, 0)) == json_type_int, "get");
	fail_unless(json_int_get(json_array_get(p, 0)) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
}

static void test_2(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_object, "type");
	fail_unless(json_type(json_object_get(p, "c")) == json_type_int, "get");
	fail_unless(json_type(json_object_get(p, "d")) == json_type_null, "get");

	fail_unless(json_int_get(json_object_get(p, "c")) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
}

START_TEST (test_single_success_1)
{
	call = create_single_request(json_string_new("test_1"), json_parser_parse("[2]"), json_int_new(1));

	origin = create_single_success_response(json_parser_parse("[2]"), json_int_new(1));

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_single_success_2)
{
	call = create_single_request(json_string_new("test_2"), json_parser_parse("{\"c\":2, \"d\":null}"), json_string_new("id"));

	origin = create_single_success_response(json_parser_parse("{\"c\":2, \"d\":null}"), json_string_new("id"));

	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_single_error_1)
{
	call = create_single_request(json_string_new("test_3"), json_parser_parse("[]"), json_string_new("id"));

	origin = create_single_error_response(create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE), json_string_new("id"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_single_error_2)
{
	call = json_object_new();
	fail_unless(call != NULL, "object_new");
	fail_unless(json_object_add(call, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(call, REQUEST_ID, json_string_new("id")) == 0, "add");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_success_1)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\"]"),
									json_parser_parse("[[2]]"),
									json_parser_parse("[1]"));

	origin = create_batched_response(	json_parser_parse("[{\"success\":[2]}]"),
										json_parser_parse("[1]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_success_2)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\"]"),
									json_parser_parse("[[2], {\"c\":2, \"d\":null}]"),
									json_parser_parse("[1, 8]"));

	origin = create_batched_response(	json_parser_parse("[{\"success\":[2]}, {\"success\":{\"c\":2, \"d\":null}}]"),
										json_parser_parse("[1, 8]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_1)
{
	call = create_batched_request(	json_parser_parse("[\"test_1\"]"),
									json_parser_parse("[[2]]"),
									json_parser_parse("[1]"));

	struct json_object *array = json_array_new();
	struct json_object *object = json_object_new();
	fail_unless(json_object_add(object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add");
	fail_unless(json_array_add(array, object) == 0, "add");
	origin = create_batched_response(array, json_parser_parse("[1]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_2)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\"]"),
									json_parser_parse("[[2], [3]]"),
									json_parser_parse("[1, 2]"));

	struct json_object *err_object = json_object_new();
	fail_unless(json_object_add(err_object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add");

	struct json_object *res_array = json_array_new();

	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":[2]}")) == 0, "add");
	fail_unless(json_array_add(res_array, err_object) == 0, "add");

	origin = create_batched_response(res_array, json_parser_parse("[1, 2]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_3)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_3\", \"test_2\"]"),
									json_parser_parse("[[2], [3], {\"c\":2, \"d\":null}]"),
									json_parser_parse("[1, 2, 3]"));

	struct json_object *err_object = json_object_new();
	fail_unless(json_object_add(err_object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add");

	struct json_object *res_array = json_array_new();

	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":[2]}")) == 0, "add");
	fail_unless(json_array_add(res_array, err_object) == 0, "add");
	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":{\"c\":2, \"d\":null}}")) == 0, "add");

	origin = create_batched_response(res_array, json_parser_parse("[1, 2, 3]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_4)
{
	call = json_array_new();
	fail_unless(call != NULL, "array_new");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_notification_1)
{
	call = create_single_request(json_string_new("test_1"), json_parser_parse("[2]"), json_null_new());

	origin = NULL;

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	json_rpc_process(jr, call, test_notification, NULL);

	event_dispatch();
}
END_TEST

START_TEST (test_notification_2)
{
	call = create_single_request(json_string_new("test_3"), json_parser_parse("[]"), json_null_new());

	origin = NULL;

	json_rpc_process(jr, call, test_notification, NULL);

	event_dispatch();
}
END_TEST

START_TEST (test_notification_3)
{
	call = json_object_new();
	fail_unless(call != NULL, "object_new");
	fail_unless(json_object_add(call, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_notification_1)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\", \"test_2\"]"),
									json_parser_parse("[[2], {\"c\":2, \"d\":null}, {\"c\":2, \"d\":null}]"),
									json_parser_parse("[1, null, \"cdcd\"]"));

	origin = create_batched_response(	json_parser_parse("[{\"success\":[2]}, {\"success\":{\"c\":2, \"d\":null}}]"),
										json_parser_parse("[1, \"cdcd\"]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_notification_2)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\", \"test_2\"]"),
									json_parser_parse("[[2], {\"c\":2, \"d\":null}, {\"c\":2, \"d\":null}]"),
									json_parser_parse("[null, null, null]"));

	origin = NULL;

	json_rpc_process(jr, call, test_notification, NULL);

	event_dispatch();
}
END_TEST


TCase *json_rpc_process_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create ("json_rpc_process");
	tcase_add_unchecked_fixture(tc, setup, teardown);

	tcase_add_test (tc, test_single_success_1);
	tcase_add_test (tc, test_single_success_2);

	tcase_add_test (tc, test_single_error_1);
	tcase_add_test (tc, test_single_error_2);

	tcase_add_test (tc, test_batched_success_1);
	tcase_add_test (tc, test_batched_success_2);

	tcase_add_test (tc, test_batched_error_1);
	tcase_add_test (tc, test_batched_error_2);
	tcase_add_test (tc, test_batched_error_3);
	tcase_add_test (tc, test_batched_error_4);

	tcase_add_test(tc, test_notification_1);
	tcase_add_test(tc, test_notification_2);
	tcase_add_test(tc, test_notification_3);

	tcase_add_test(tc, test_batched_notification_1);
	tcase_add_test(tc, test_batched_notification_2);

	return tc;
}
