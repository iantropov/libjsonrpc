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

void json_rpc_process_request(struct json_rpc *jr, struct json_object *obj, json_rpc_result user_cb, void *cb_arg);

int json_rpc_preprocess_request(struct json_rpc *jr, struct json_object *req, json_rpc_result res_cb, void *cb_arg);
void json_rpc_process_response(struct json_rpc *jr, struct json_object *resp);

int json_rpc_is_response(struct json_object *obj);

#endif /* JSON_RPC_H_ */
