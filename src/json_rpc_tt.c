/*
 * json_rpc_tt.c
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#include "json_rpc_tt_internal.h"

#include "json_rpc.h"
#include "log.h"

static void json_rpc_tt_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	if (res == NULL) {
		log_warn("%s : result_object == NULL", __func__);
		return;
	}

	struct json_rpc_tt *jt = arg;

	if (jt->write(jt, res) == -1)
		log_warn("%s : sending of JSON-RPC response failed", __func__);

	json_ref_put(res);
}

static void json_rpc_tt_read(struct json_rpc_tt *jt, struct json_object *obj)
{
	if (json_rpc_is_response(obj))
		json_rpc_process_response(jt->jr, obj);
	else
		json_rpc_process_request(jt->jr, obj, json_rpc_tt_result, jt);
}

void json_rpc_tt_free(struct json_rpc_tt *jt)
{
	jt->free(jt);

	free(jt);
}

int json_rpc_tt_send(struct json_rpc_tt *jt, struct json_object *req, json_rpc_result res_cb, void *arg)
{
	if (json_rpc_preprocess_request(jt->jr, req, res_cb, arg) == -1)
		return -1;

	return jt->write(jt, req);
}

struct json_rpc_tt *json_rpc_tt_new(struct json_rpc *jr)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)calloc(1, sizeof(struct json_rpc_tt));
	if (jt == NULL)
		return NULL;

	jt->jr = jr;
	jt->read = json_rpc_tt_read;

	return jt;
}
