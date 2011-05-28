/*
 * json_rpc_tt_ws.c
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#include "json_rpc_tt_internal.h"

struct jrpc_ws {
	struct ws_connection *conn;

	ws_error_cb e_cb;
	void *arg;
};

static void ws_messagecb(struct ws_connection *conn, u_char *mess, void *arg)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_ws *jw = (struct jrpc_ws *)jt->impl;

	struct json_object *obj = json_parser_parse((char *)mess);

	if (obj == NULL) {
		if (ws_connection_send_message(jw->conn, (u_char *)JSON_RPC_PARSE_ERROR) == -1)
			log_warn("%s : sending of json_parse_error failed", __func__);
		return;
	}

	jt->read(jt, obj);
}

static int tt_ws_write(struct json_rpc_tt *jt, struct json_object *obj)
{
	struct jrpc_ws *jw = (struct jrpc_ws *)jt->impl;

	char *str = json_to_string(obj);

	int ret = ws_connection_send_message(jw->conn, (u_char *)str);

	free(str);

	return ret;
}

static void tt_ws_free(struct json_rpc_tt *jt)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_ws *jw = (struct jrpc_ws *)jt->impl;

	ws_connection_send_close(jw->conn);
	ws_connection_free(jw->conn);

	free(jw);
	free(jt);
}

struct json_rpc_tt *json_rpc_tt_ws_new(struct json_rpc *jr, struct ws_connection *conn, ws_error_cb e_cb, void *arg)
{
	struct json_rpc_tt *jt = json_rpc_tt_new(jr);
	if (jt == NULL)
		return NULL;

	struct jrpc_ws *jw = (struct jrpc_ws *)calloc(1, sizeof(struct jrpc_ws));
	if (jw == NULL) {
		free(jt);
		return NULL;
	}

	jw->conn = conn;
	jw->e_cb = e_cb;
	jw->arg = arg;

	jt->impl = jw;
	jt->write_request = tt_ws_write;
	jt->write_response = tt_ws_write;
	jt->free = tt_ws_free;

	ws_set_cbs(conn, NULL, ws_messagecb, ws_errorcb, wj);

	return jt;
}
