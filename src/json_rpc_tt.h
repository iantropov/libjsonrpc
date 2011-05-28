/*
 * json_rpc_tt.h
 *
 *  Created on: May 28, 2011
 *      Author: ant
 */

#ifndef JSON_RPC_TT_H_
#define JSON_RPC_TT_H_

struct json_rpc_tt;

struct json_rpc_tt *json_rpc_tt_ws_new(struct json_rpc *jr, struct ws_connection *conn, ws_error_cb e_cb, void *arg);
struct json_rpc_tt *json_rpc_tt_tcp_new(struct json_rpc *jr, struct bufevent *bufev, everrcb ecb, void *arg);
struct json_rpc_tt *json_rpc_tt_http_new(struct json_rpc *jr, struct evhttp *eh, char *uri);

int json_rpc_tt_send(struct json_rpc_tt *jt, struct json_object *req, json_rpc_result res_cb, void *arg);
void json_rpc_tt_free(struct json_rpc_tt *jt);

#endif /* JSON_RPC_TT_H_ */
