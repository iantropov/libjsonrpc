/*
 * json_rpc_over_http.c
 *
 *  Created on: Mar 8, 2011
 *      Author: ant
 */

#include "json_rpc.h"
#include "json_parser.h"
#include "string_functions.h"

#include <string.h>

#define HTTP_PARSE_ERROR_REASON "Parse error."
#define HTTP_INTERNAL_ERROR_REASON "Internal error."
#define HTTP_SUCCESS_REASON "OK"

#define HTTP_INVALID_REQUEST_STATUS 400
#define HTTP_METHOD_NOT_FOUND_STATUS 404
#define HTTP_INTERNAL_ERROR_STATUS 500
#define HTTP_PARSE_ERROR_STATUS 500
#define HTTP_SUCCESS_STATUS 200

#define HTTP_CONTENT_TYPE_KEY "Content-Type"
#define HTTP_CONTENT_TYPE_VALUE "application/json-rpc"

static int get_status_by_code(int code)
{
	int status;
	switch (code) {
		case ERROR_INVALID_REQUEST_CODE :
			status = HTTP_INVALID_REQUEST_STATUS;
			break;
		case ERROR_METHOD_NOT_FOUND_CODE :
			status = HTTP_METHOD_NOT_FOUND_STATUS;
			break;
		case ERROR_INTERNAL_CODE :
			status = HTTP_INTERNAL_ERROR_STATUS;
			break;
		default : status = -1;
	}

	return status;
}

static int get_status_and_reason(struct json_object *obj, int *status, char **reason)
{
	char *result_reason;
	int result_status;
	if (json_type(obj) == json_type_array || json_object_get(obj, JSON_RPC_ERROR_MEMBER) == NULL) {
		result_status = HTTP_SUCCESS_STATUS;
		result_reason = HTTP_SUCCESS_REASON;
	} else {
		int result_code = json_int_get(json_object_get(json_object_get(obj, JSON_RPC_ERROR_MEMBER), JSON_RPC_ERROR_CODE_MEMBER));
		result_reason = string_copy(json_string_get(json_object_get(json_object_get(obj, JSON_RPC_ERROR_MEMBER), JSON_RPC_ERROR_MESSAGE_MEMBER)));
		result_status = get_status_by_code(result_code);
		if (result_reason == NULL || result_status == -1)
			return -1;
	}

	*reason = result_reason;
	*status = result_status;

	return 0;
}

static void send_http_reply(struct evhttp_request *req, int status, char *reason, char *body)
{
	struct evbuffer *res = evbuffer_new();
	evbuffer_add(res, body, strlen(body));
	evhttp_add_header(req->output_headers, HTTP_CONTENT_TYPE_KEY, HTTP_CONTENT_TYPE_VALUE);
	evhttp_send_reply(req, status, reason, res);
	evbuffer_free(res);
}

static void send_reply(struct evhttp_request *req, struct json_object *obj)
{
	char *result_reason;
	int result_status;

	if (get_status_and_reason(obj, &result_status, &result_reason) == -1)
		send_http_reply(req, HTTP_INTERNAL_ERROR_STATUS, HTTP_INTERNAL_ERROR_REASON, JSON_RPC_INTERNAL_ERROR);
	else {
		char *obj_str = json_to_string(obj);
		send_http_reply(req, result_status, result_reason, obj_str);
		free(obj_str);
	}
}

static void jrpc_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	struct evhttp_request *req = (struct evhttp_request *)arg;

	if (res == NULL)
		send_http_reply(req, HTTP_INTERNAL_ERROR_STATUS, HTTP_INTERNAL_ERROR_REASON, JSON_RPC_INTERNAL_ERROR);
	else
		send_reply(req, res);

	json_ref_put(res);
}

static void json_rpc_call(struct evhttp_request *req, void *arg)
{
	struct json_rpc *jr = (struct json_rpc *)arg;
	char *buf = (char *)req->input_buffer->buffer;

	u_char c = req->input_buffer->buffer[req->input_buffer->off];
	req->input_buffer->buffer[req->input_buffer->off] = '\0';
	struct json_object *obj = json_parser_parse(buf);
	req->input_buffer->buffer[req->input_buffer->off] = c;

	if (obj == NULL)
		send_http_reply(req, HTTP_PARSE_ERROR_STATUS, HTTP_PARSE_ERROR_REASON, JSON_RPC_PARSE_ERROR);
	else
		json_rpc_process_request(jr, obj, jrpc_result, (void *)req);
}

void evhttp_set_json_rpc(struct evhttp *eh, char *uri, struct json_rpc *jr)
{
	evhttp_set_cb(eh, uri, json_rpc_call, (void *)jr);
}
