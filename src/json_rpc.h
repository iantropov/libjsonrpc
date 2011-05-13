/*
 * json_rpc.h
 *
 *  Created on: Mar 4, 2011
 *      Author: ant
 */

#ifndef JSON_RPC_H_
#define JSON_RPC_H_

#include "json.h"

#include <event0/evhttp.h>
#include <event0/bufevent.h>
#include <ws/web_sockets.h>

#define JSON_RPC_PARSE_ERROR "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":-32700, \"message\":\"Parse error.\"}, \"id\": null}"
#define JSON_RPC_INTERNAL_ERROR "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":-32603, \"message\":\"Internal error.\"}, \"id\": null}"

#define JSON_RPC_ERROR_MEMBER "error"
#define JSON_RPC_ERROR_CODE_MEMBER "code"
#define JSON_RPC_ERROR_MESSAGE_MEMBER "message"

#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL_CODE -32603

struct json_rpc;
struct json_rpc_request;

typedef void (*json_rpc_method)(struct json_rpc_request *req, struct json_object *obj, void *arg);
typedef void (*json_rpc_result)(struct json_rpc *jr, struct json_object *obj, void *arg);

struct json_rpc *json_rpc_new();
void json_rpc_free(struct json_rpc *jr);

int json_rpc_add_method(struct json_rpc *jr, char *name, json_rpc_method method, void *arg);
void json_rpc_del_method(struct json_rpc *jr, char *name);

void json_rpc_return(struct json_rpc_request *jr, struct json_object *res);

void json_rpc_process(struct json_rpc *jr, struct json_object *obj, json_rpc_result user_cb, void *cb_arg);


void evhttp_set_json_rpc(struct evhttp *eh, char *uri, struct json_rpc *jr);


struct bufevent_jrpc;

void bufevent_json_rpc_free(struct bufevent_jrpc *bj);
void bufevent_json_rpc_send(struct bufevent_jrpc *bj, struct json_object *obj);
struct bufevent_jrpc *bufevent_json_rpc_new(struct bufevent *bufev,
		struct json_rpc *jr, everrcb bufev_err_cb, json_rpc_result response_cb, void *cb_arg);


struct ws_jrpc;

int ws_json_rpc_send(struct ws_jrpc *wj, struct json_object *obj);
struct ws_jrpc *ws_json_rpc_new(struct ws_connection *conn,
		struct json_rpc *jr, json_rpc_result result_cb, ws_error_cb error_cb, void *cb_arg);
void ws_json_rpc_free(struct ws_jrpc *wj);

#endif /* JSON_RPC_H_ */
