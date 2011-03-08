/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "util/list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <event.h>

#include "json/json.h"
#include "json_rpc/json_rpc.h"
#include "json_parser/json_parser.h"

struct my_list {
	int value;
	unsigned long b;
	struct list_head list;
};

#define CHECK_INT 5
#define CHECK_STR "cdcd"

#define REQUEST_ID "id"
#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"
#define REQUEST_VERSION "jsonrpc"

#define RESPONSE_ERROR "error"
#define RESPONSE_ERROR_MESSAGE "message"
#define RESPONSE_ERROR_CODE "code"
#define RESPONSE_RESULT "result"

#define JSON_RPC_VERSION "2.0"

#define ERROR_INVALID_REQUEST_MESSAGE "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND_MESSAGE "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL_MESSAGE "Internal error"
#define ERROR_INTERNAL_CODE -32603


static void fail_unless(boolean b, char *s)
{
	if (!b)
		printf("%s\n", s);
}

void test_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	struct json_object *origin = *((struct json_object **)arg);
	fail_unless(json_equals(res, *((struct json_object **)arg)) == 0, "invalid result");
//	if (json_int_get(json_object_get(json_array_get(res, 1), "id")) != 2)
		json_ref_put(res);
	event_loopbreak();
}

struct json_object *create_single_request(struct json_object *method, struct json_object *params, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_PARAMS, json_ref_get(params)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_ID, json_ref_get(id)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_METHOD, json_ref_get(method)) == 0, "add");

	return obj;
}

struct json_object *create_single_success_response(struct json_object *result, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_ID, json_ref_get(id)) == 0, "add");
	fail_unless(json_object_add(obj, RESPONSE_RESULT, json_ref_get(result)) == 0, "add");

	return obj;
}

struct json_object *create_error(char *message, int code)
{
	struct json_object *j_err = json_object_new();
	fail_unless(j_err != NULL, "object_new");
	fail_unless(json_object_add(j_err, RESPONSE_ERROR_MESSAGE, json_string_new(message)) == 0, "add");
	fail_unless(json_object_add(j_err, RESPONSE_ERROR_CODE, json_int_new(code)) == 0, "add");

	return j_err;
}

struct json_object *create_single_error_response(struct json_object *j_err, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_ID, json_ref_get(id)) == 0, "add");
	fail_unless(json_object_add(obj, RESPONSE_ERROR, json_ref_get(j_err)) == 0, "add");

	return obj;
}

struct json_object *create_batched_request(struct json_object *methods, struct json_object *params, struct json_object *ids)
{
	struct json_object *ar = json_array_new();
	fail_unless(ar != NULL, "array_new");
	int i, len = json_array_length(methods);
	for (i = 0; i < len; i++)
		json_array_add(ar, create_single_request(	json_array_get(methods, i),
													json_array_get(params, i),
													json_array_get(ids, i)));



	json_ref_put(methods);
	json_ref_put(params);
	json_ref_put(ids);
	return ar;
}

struct json_object *create_batched_response(struct json_object *results,struct json_object *ids)
{
	struct json_object *ar = json_array_new();
	fail_unless(ar != NULL, "array_new");
	int i, len = json_array_length(results);
	struct json_object *entry;
	for (i = 0; i < len; i++) {
		entry = json_array_get(results, i);
		if (json_object_get(entry, "success"))
			json_array_add(ar, create_single_success_response(json_object_get(entry, "success"),json_array_get(ids, i)));
		else
			json_array_add(ar, create_single_error_response(json_object_get(entry, "error"),json_array_get(ids, i)));

	}

	json_ref_put(results);
	json_ref_put(ids);
	return ar;
}

void test_1(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_array, "type");
	fail_unless(json_array_length(p) == 1, "length");
	fail_unless(json_type(json_array_get(p, 0)) == json_type_int, "get");
	fail_unless(json_int_get(json_array_get(p, 0)) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
}

void test_2(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_object, "type");
	fail_unless(json_type(json_object_get(p, "c")) == json_type_int, "get");
	fail_unless(json_type(json_object_get(p, "d")) == json_type_null, "get");

	fail_unless(json_int_get(json_object_get(p, "c")) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
}

void v2()
{
	struct json_object *call, *origin;
	struct json_rpc *jr;

	event_init();
	jr = json_rpc_init(test_result, &origin);
	fail_unless(jr != NULL, "init error");

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

	json_rpc_process(jr, call);

	event_dispatch();

	json_ref_put(call);
	json_ref_put(origin);
	json_rpc_destroy(jr);
}

void v1()
{
	struct json_object *call, *origin;
	struct json_rpc *jr;

	event_init();
	jr = json_rpc_init(test_result, &origin);
	fail_unless(jr != NULL, "init error");

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\"]"),
									json_parser_parse("[[2]]"),
									json_parser_parse("[1]"));

//	struct json_object *err_object = json_object_new();
//	fail_unless(json_object_add(err_object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add_3");

	struct json_object *res_array = json_array_new();

	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":[2]}")) == 0, "add_1");
//	fail_unless(json_array_add(res_array, err_object) == 0, "add_2");

	origin = create_batched_response(res_array, json_parser_parse("[1]"));

	json_rpc_process(jr, call);

	event_dispatch();

	json_ref_put(call);
	json_ref_put(origin);
	json_rpc_destroy(jr);
}

int main()
{
	v2();

	return 0;
}

