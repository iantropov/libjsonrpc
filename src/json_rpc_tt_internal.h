/*
 * json_rpc_tt_internal.h
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#ifndef JSON_RPC_TT_INTERNAL_H_
#define JSON_RPC_TT_INTERNAL_H_

struct json_rpc_tt;

typedef int (*tt_write_cb)(struct json_rpc_tt *jt, struct json_object *obj);
typedef void (*tt_read_cb)(struct json_rpc_tt *jt, struct json_object *obj);
typedef void (*tt_free_cb)(struct json_rpc_tt *jt);

struct json_rpc_tt {
	struct json_rpc *jr;

	void *impl;

	tt_write_cb write_request;
	tt_write_cb write_response;
	tt_read_cb read;
	tt_free_cb free;
};

struct json_rpc_tt *json_rpc_tt_new(struct json_rpc *jr);

#endif /* JSON_RPC_TT_INTERNAL_H_ */
