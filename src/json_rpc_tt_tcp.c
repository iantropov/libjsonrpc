/*
 * json_rpc_tt_tcp.c
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#include "json_rpc.h"
#include "json_parser.h"
#include "log.h"

#include "json_rpc_tt_internal.h"

struct jrpc_bufevent {
	struct bufevent *bufev;
	struct json_parser *jp;

	everrcb err_cb;
	void *cb_arg;
};

static void bufevent_readcb(struct bufevent *bufev, void *arg)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)jt->impl;

	struct evbuffer *buf = bufevent_get_input(bufev);
	json_parser_process(jb->jp, (char *)EVBUFFER_DATA(buf), (char *)EVBUFFER_DATA(buf) + EVBUFFER_LENGTH(buf));
}

static void bufevent_errorcb(struct bufevent *bufev, short what, void *arg)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)jt->impl;

	jb->err_cb(jb->bufev, what, jb->cb_arg);
}

static int tt_bufevent_write(struct json_rpc_tt *jt, struct json_object *obj)
{
	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)jt->impl;

	char *json_str = json_to_string(obj);

	int ret = bufevent_write(jb->bufev, json_str, strlen(json_str));
	if (ret == -1)
		log_info("%s : bufevent_write_failed", __func__);

	free(json_str);

	return ret;
}

static void json_objectcb(struct json_parser *jp, struct json_object *obj, void *arg)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	jt->read(jt, obj);
}

static void json_errorcb(struct json_parser *jp, short what, void *arg)
{
	struct json_rpc_tt *jt = (struct json_rpc_tt *)arg;
	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)jt->impl;

	if (bufevent_write(jb->bufev, JSON_RPC_PARSE_ERROR, strlen(JSON_RPC_PARSE_ERROR)) == -1)
		log_warn("%s : bufevent write of JSON-RPC_PARSE_ERROR failed");
}

static void tt_bufevent_free(struct json_rpc_tt *jt)
{
	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)jt->impl;

	json_parser_free(jb->jp);
	bufevent_free(jb->bufev);

	free(jb);
	free(jt);
}

struct json_rpc_tt *json_rpc_tt_tcp_new(struct json_rpc *jr, struct bufevent *bufev, everrcb ecb, void *arg)
{
	struct json_rpc_tt *jt = json_rpc_tt_new(jr);
	if (jt == NULL)
		return NULL;

	struct jrpc_bufevent *jb = (struct jrpc_bufevent *)malloc(sizeof(struct jrpc_bufevent));
	if (jb == NULL) {
		free(jt);
		return NULL;
	}

	struct json_parser *jp = json_parser_new(json_objectcb, json_errorcb, jt);
	if (jp == NULL) {
		free(jb);
		free(jt);
		return NULL;
	}

	jb->bufev = bufev;
	jb->jp = jp;
	jb->err_cb = ecb;
	jb->cb_arg = arg;

	jt->impl = jb;
	jt->free = tt_bufevent_free;
	jt->write = tt_bufevent_write;

	bufevent_setcb(bufev, bufevent_readcb, NULL, bufevent_errorcb, jt);

	return jt;
}
