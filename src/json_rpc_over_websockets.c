/*
 * json_rpc_over_websockets.c
 *
 *  Created on: May 12, 2011
 *      Author: ant
 */

#include <ws/web_sockets.h>
#include "json_rpc.h"
#include "log.h"
#include "json_parser.h"

struct ws_jrpc {
	struct json_rpc *jr;
	struct ws_connection *conn;

	json_rpc_result result_cb;
	ws_error_cb error_cb;
	void *cb_arg;
};

static int is_json_rpc_response(struct json_object *obj)
{
	if (json_type(obj) != json_type_object)
		return 0;

	if (json_object_get(obj, "result") == NULL && json_object_get(obj, "error") == NULL)
		return 0;

	return 1;
}

static void ws_errorcb(struct ws_connection *conn, short what, void *arg)
{
	struct ws_jrpc *wj = arg;

	if (wj->error_cb != NULL)
		wj->error_cb(wj->conn, what, wj->cb_arg);
}

static int send_json(struct ws_connection *conn, struct json_object *obj)
{
	char *str = json_to_string(obj);

	int ret = ws_send_message(conn, (u_char *)str);

	free(str);

	return ret;
}

int ws_json_rpc_send(struct ws_jrpc *wj, struct json_object *obj)
{
	return send_json(wj->conn, obj);
}

static void jrpc_result(struct json_rpc *jr, struct json_object *obj, void *arg)
{
	struct ws_jrpc *wj = arg;

	if (obj == NULL) {
		log_warn("%s : json_rpc_process returns NULL", __func__);
		return;
	}

	if (send_json(wj->conn, obj) == -1)
		log_warn("%s : sending of the response failed", __func__);

	json_ref_put(obj);
}

static void ws_messagecb(struct ws_connection *conn, u_char *mess, void *arg)
{
	struct ws_jrpc *wj = arg;

	struct json_object *obj = json_parser_parse((char *)mess);

	if (obj == NULL) {
		if (ws_send_message(wj->conn, (u_char *)JSON_RPC_PARSE_ERROR) == -1)
			log_warn("%s : sending of json_parse_error failed", __func__);
		return;
	}

	if (!is_json_rpc_response(obj))
		json_rpc_process(wj->jr, obj, jrpc_result, wj);
	else if (wj->result_cb != NULL)
		wj->result_cb(wj->jr, obj, wj->cb_arg);
}

struct ws_jrpc *ws_json_rpc_new(struct ws_connection *conn,
		struct json_rpc *jr, json_rpc_result result_cb, ws_error_cb error_cb, void *cb_arg)
{
	struct ws_jrpc *wj = (struct ws_jrpc *)calloc(1, sizeof(struct ws_jrpc));
	if (wj == NULL) {
		log_info("%s : malloc failed", __func__);
		return NULL;
	}

	wj->jr = jr;
	wj->conn = conn;
	wj->result_cb = result_cb;
	wj->error_cb = error_cb;
	wj->cb_arg = cb_arg;

	ws_set_cbs(conn, NULL, ws_messagecb, ws_errorcb, wj);

	return wj;
}

void ws_json_rpc_free(struct ws_jrpc *wj)
{
	ws_send_close(wj->conn);
	ws_free(wj->conn);
	free(wj);
}
