/*
 * json_rpc_tt_http.c
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#include "json_rpc_tt_internal.h"
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

static int get_status_by_code(int code);
static int get_status_and_reason(struct json_object *obj, int *status, char **reason);

struct jrpc_http {
	struct evhttp *eh;
	char *uri;
};

static void send_http_reply(struct evhttp_request *req, int status, char *reason, char *body)
{
	struct evbuffer *res = evbuffer_new();
	evbuffer_add(res, body, strlen(body));
	evhttp_add_header(req->output_headers, HTTP_CONTENT_TYPE_KEY, HTTP_CONTENT_TYPE_VALUE);
	evhttp_send_reply(req, status, reason, res);
	evbuffer_free(res);
}

static int send_reply(struct evhttp_request *req, struct json_object *obj)
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

	return 0;
}

static struct json_object *get_json(struct evbuffer *buf)
{
	u_char c = buf->buffer[buf->off];
	buf->buffer[buf->off] = '\0';
	struct json_object *obj = json_parser_parse((char *)buf->buffer);
	buf->buffer[buf->off] = c;

	return obj;
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
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_http *jh = (struct jrpc_http *)jt->impl;

	struct json_object *obj = get_json(req->input_buffer);

	if (obj == NULL)
		send_http_reply(req, HTTP_PARSE_ERROR_STATUS, HTTP_PARSE_ERROR_REASON, JSON_RPC_PARSE_ERROR);
	else
		json_rpc_process_request(jt->jr, obj, jrpc_result, (void *)req);
}

static int tt_http_write(struct json_rpc_tt *jt, struct json_object *obj)
{
	return 0;
}

static void http_free(struct json_rpc_tt *jt)
{
	struct jrpc_http *jh = (struct jrpc_http *)jt->impl;

	evhttp_del_cb(jh->eh, jh->uri);
	free(jh->uri);
	free(jh);
	free(jt);
}

struct json_rpc_tt *json_rpc_tt_http_new(struct json_rpc *jr, struct evhttp *eh, char *uri)
{
	struct json_rpc_tt *jt = json_rpc_tt_new(jr);
	if (jt == NULL)
		return NULL;

	struct jrpc_http *jh = (struct jrpc_http *)malloc(sizeof(struct jrpc_http));
	if (jh == NULL) {
		free(jt);
		return NULL;
	}

	jh->uri = strdup(uri);
	if (jh->uri == NULL) {
		free(jt);
		free(jh);
		return NULL;
	}

	jh->eh = eh;

	jt->impl = jh;
	jt->write = tt_http_write;
	jt->free = http_free;

	evhttp_set_cb(eh, uri, json_rpc_call, jt);

	return jt;
}

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
