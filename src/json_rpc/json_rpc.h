/*
 * json_rpc.h
 *
 *  Created on: Mar 4, 2011
 *      Author: ant
 */

#ifndef JSON_RPC_H_
#define JSON_RPC_H_

#include "../json/json.h"

struct json_rpc;

typedef void (json_rpc_method)(struct json_rpc *jr, struct json_object *obj, void *arg);

struct json_rpc *json_rpc_init(json_rpc_method *result_method, void *arg);
void json_rpc_destroy(struct json_rpc *jr);

int json_rpc_add_method(struct json_rpc *jr, char *name, json_rpc_method *meth, void *arg);
void json_rpc_del_method(struct json_rpc *jr, char *name);

void json_rpc_return(struct json_rpc *jr, struct json_object *res);

void json_rpc_process(struct json_rpc *jr, struct json_object *obj);

#endif /* JSON_RPC_H_ */
