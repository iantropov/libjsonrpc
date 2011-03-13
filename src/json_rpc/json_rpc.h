/*
 * json_rpc.h
 *
 *  Created on: Mar 4, 2011
 *      Author: ant
 */

#ifndef JSON_RPC_H_
#define JSON_RPC_H_

#include "../json/json.h"

#include <evhttp.h>

struct json_rpc;

typedef void (json_rpc_method)(struct json_rpc *jr, struct json_object *obj, void *arg);

struct json_rpc *json_rpc_init();
void json_rpc_destroy(struct json_rpc *jr);

int json_rpc_add_method(struct json_rpc *jr, char *name, json_rpc_method *method, void *arg);
void json_rpc_del_method(struct json_rpc *jr, char *name);

void json_rpc_return(struct json_rpc *jr, struct json_object *res);

void json_rpc_process(struct json_rpc *jr, struct json_object *obj, json_rpc_method *user_cb, void *user_cb_arg);


void evhttp_set_json_rpc(struct evhttp *eh, char *uri, struct json_rpc *jr);

#endif /* JSON_RPC_H_ */
