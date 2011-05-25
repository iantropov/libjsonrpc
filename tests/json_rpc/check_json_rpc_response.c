#include "check.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../src/json_rpc.h"
#include "../../src/json_parser.h"

#define SINGLE_REQUEST "{\"jsonrpc\":\"2.0\", \"method\":\"test1\", \"params\":[2], \"id\":2}"
#define SINGLE_RESPONSE "{\"jsonrpc\":\"2.0\", \"result\":[2], \"id\":2}"
#define BATCHED_REQUEST "[{\"jsonrpc\":\"2.0\", \"method\":\"test1\", \"params\":[2], \"id\":2}, {\"jsonrpc\":\"2.0\", \"method\":\"test1\", \"params\":[2], \"id\":3}]"
#define BATCHED_RESPONSE "[{\"jsonrpc\":\"2.0\", \"result\":[2], \"id\":3}, {\"jsonrpc\":\"2.0\", \"result\":[2], \"id\":2}]"

static struct json_rpc *__jr;

static int __call_count;
static int __waiting_call_count;

static void setup()
{
	__call_count = __waiting_call_count = 0;

	__jr = json_rpc_new();
	fail_unless(__jr != NULL, "json_rpc_new failed");
}

static void teardown()
{
	fail_unless(__call_count == __waiting_call_count, "not all cbs are called");

	json_rpc_free(__jr);
}

static void result_cb(struct json_rpc *jr, struct json_object *resp, void *arg)
{
	struct json_object *orig = (struct json_object *)arg;

	fail_unless(json_equals(resp, orig) == 0, "unsuspected response");

	json_ref_put(resp);

	__call_count++;
}

START_TEST(test_single_0)
{
	struct json_object *req = json_parser_parse(SINGLE_REQUEST);
	struct json_object *resp = json_parser_parse(SINGLE_RESPONSE);
	fail_unless(req != NULL && resp != NULL, "parse_error");

	fail_unless(json_rpc_preprocess_request(__jr, req, result_cb, (void *)resp) == 0, "save_result_cb failed");

	json_ref_put(req);

	json_rpc_process_response(__jr, resp);

	__waiting_call_count = 1;
}
END_TEST

START_TEST(test_single_1)
{
	struct json_object *req = json_parser_parse(SINGLE_REQUEST);
	struct json_object *resp = json_parser_parse("{\"cd\":123, \"id\":2}");
	fail_unless(req != NULL && resp != NULL, "parse_error");

	fail_unless(json_rpc_preprocess_request(__jr, req, result_cb, (void *)resp) == 0, "save_result_cb failed");

	json_ref_put(req);

	json_rpc_process_response(__jr, resp);

	__waiting_call_count = 0;

	json_ref_put(resp);
}
END_TEST

START_TEST(test_batched_0)
{
	struct json_object *req = json_parser_parse(BATCHED_REQUEST);
	struct json_object *resp = json_parser_parse(BATCHED_RESPONSE);
	fail_unless(req != NULL && resp != NULL, "parse_error");

	fail_unless(json_rpc_preprocess_request(__jr, req, result_cb, (void *)resp) == 0, "save_result_cb failed");

	json_ref_put(req);

	json_rpc_process_response(__jr, resp);

	__waiting_call_count = 1;

	json_ref_put(resp);
}
END_TEST

START_TEST(test_batched_1)
{
	struct json_object *req = json_parser_parse("[{\"jsonrpc\":\"2.0\", \"method\":\"test1\",\"params\":[2],\"id\":2},"
			"{\"jsonrpc\":\"2.0\",\"method\":\"test1\",\"params\":[2],\"id\":3},"
			"{\"jsonrpc\":\"2.0\",\"method\":\"test1\",\"params\":[2]}]");
	struct json_object *resp = json_parser_parse(BATCHED_RESPONSE);
	fail_unless(req != NULL && resp != NULL, "parse_error");

	fail_unless(json_rpc_preprocess_request(__jr, req, result_cb, (void *)resp) == 0, "save_result_cb failed");

	json_ref_put(req);

	json_rpc_process_response(__jr, resp);

	__waiting_call_count = 1;

	json_ref_put(resp);
}
END_TEST

START_TEST(test_is_response_success_0)
{
	struct json_object *obj_1 = json_parser_parse(SINGLE_RESPONSE);
	struct json_object *obj_2 = json_parser_parse(BATCHED_RESPONSE);

	fail_unless(json_rpc_is_response(obj_1) == 1, "is_response failed");
	fail_unless(json_rpc_is_response(obj_2) == 1, "is_response failed");

	json_ref_put(obj_1);
	json_ref_put(obj_2);
}
END_TEST

START_TEST(test_is_response_success_1)
{
	struct json_object *obj_3 = json_parser_parse(JSON_RPC_PARSE_ERROR);
	struct json_object *obj_4 = json_parser_parse(JSON_RPC_INTERNAL_ERROR);

	fail_unless(json_rpc_is_response(obj_3) == 1, "is_response failed");
	fail_unless(json_rpc_is_response(obj_4) == 1, "is_response failed");

	json_ref_put(obj_3);
	json_ref_put(obj_4);
}
END_TEST

START_TEST(test_is_response_error)
{
	struct json_object *obj_1 = json_parser_parse(SINGLE_REQUEST);
	struct json_object *obj_2 = json_parser_parse(BATCHED_REQUEST);

	fail_unless(json_rpc_is_response(obj_1) == 0, "is_response failed");
	fail_unless(json_rpc_is_response(obj_2) == 0, "is_response failed");

	json_ref_put(obj_1);
	json_ref_put(obj_2);
}
END_TEST

TCase *json_rpc_response_tcase()
{
	TCase *tc = tcase_create ("json_rpc_response");
	tcase_add_checked_fixture(tc, setup, teardown);

	tcase_add_test(tc, test_single_0);
	tcase_add_test(tc, test_single_1);

	tcase_add_test(tc, test_batched_0);
	tcase_add_test(tc, test_batched_1);

	tcase_add_test(tc, test_is_response_success_0);
	tcase_add_test(tc, test_is_response_success_1);
	tcase_add_test(tc, test_is_response_error);

	return tc;
}
