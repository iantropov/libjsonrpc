/*
 * json_rpc_over_bufevent.c
 *
 *  Created on: May 10, 2011
 *      Author: ant
 */

#include "json_rpc.h"
#include "json_parser.h"
#include "log.h"

struct bufevent_jrpc {
	struct json_rpc *jr;
	struct json_parser *jp;
	struct bufevent *bufev;

	everrcb bufev_err_cb;
	void *cb_arg;
};

static void bufevent_readcb(struct bufevent *bufev, void *arg)
{
	struct bufevent_jrpc *bj = arg;

	struct evbuffer *buf = bufevent_get_input(bufev);
	json_parser_process(bj->jp, (char *)EVBUFFER_DATA(buf), (char *)EVBUFFER_DATA(buf) + EVBUFFER_LENGTH(buf));
}

static void bufevent_errorcb(struct bufevent *bufev, short what, void *arg)
{
	struct bufevent_jrpc *bj = arg;

	bj->bufev_err_cb(bufev, what, bj->cb_arg);
}

static int send_json(struct bufevent *bufev, struct json_object *obj)
{
	char *json_str = json_to_string(obj);

	int ret = bufevent_write(bufev, json_str, strlen(json_str));
	if (ret == -1)
		log_info("%s : bufevent_write_failed", __func__);

	free(json_str);

	return ret;
}

static void json_rpc_resultcb(struct json_rpc *jr, struct json_object *res, void *arg)
{
	if (res == NULL) {
		log_warn("%s : result_object == NULL", __func__);
		return;
	}

	struct bufevent_jrpc *bj = arg;
	send_json(bj->bufev, res);

	json_ref_put(res);
}

static void json_objectcb(struct json_parser *jp, struct json_object *obj, void *arg)
{
	struct bufevent_jrpc *bj = (struct bufevent_jrpc *)arg;

	if (json_rpc_is_response(obj))
		json_rpc_process_response(bj->jr, obj);
	else
		json_rpc_process_request(bj->jr, obj, json_rpc_resultcb, arg);
}

static void json_errorcb(struct json_parser *jp, short what, void *arg)
{
	struct bufevent_jrpc *tj = (struct bufevent_jrpc *)arg;

	bufevent_write(tj->bufev, JSON_RPC_PARSE_ERROR, strlen(JSON_RPC_PARSE_ERROR));
}

int bufevent_json_rpc_send(struct bufevent_jrpc *bjrpc, struct json_object *req, json_rpc_result res_cb, void *arg)
{
	if (json_rpc_preprocess_request(bjrpc->jr, req, res_cb, arg) == -1)
		return -1;

	return send_json(bjrpc->bufev, req);
}

struct bufevent_jrpc *bufevent_json_rpc_new(struct bufevent *bufev,
		struct json_rpc *jr, everrcb bufev_err_cb, void *cb_arg)
{
	struct bufevent_jrpc *bj = (struct bufevent_jrpc *)malloc(sizeof(struct bufevent_jrpc));
	if (bj == NULL)
		return NULL;

	struct json_parser *jp = json_parser_new(json_objectcb, json_errorcb, bj);
	if (jp == NULL) {
		free(bj);
		return NULL;
	}

	bj->jp = jp;
	bj->jr = jr;
	bj->bufev = bufev;
	bj->bufev_err_cb = bufev_err_cb;
	bj->cb_arg = cb_arg;

	bufevent_setcb(bufev, bufevent_readcb, NULL, bufevent_errorcb, bj);

	return bj;
}

void bufevent_json_rpc_free(struct bufevent_jrpc *bj)
{
	json_parser_free(bj->jp);

	bufevent_free(bj->bufev);

	free(bj);
}
