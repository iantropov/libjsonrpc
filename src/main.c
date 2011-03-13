/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <event.h>
#include <evhttp.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>


#include "json.h"
#include "json_rpc.h"
#include "json_parser.h"
#include "https.h"

struct my_list {
	int value;
	unsigned long b;
	struct list_head list;
};

#define CHECK_INT 5
#define CHECK_STR "cdcd"

#define REQUEST_ID "id"
#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"
#define REQUEST_VERSION "jsonrpc"

#define RESPONSE_ERROR "error"
#define RESPONSE_ERROR_MESSAGE "message"
#define RESPONSE_ERROR_CODE "code"
#define RESPONSE_RESULT "result"

#define JSON_RPC_VERSION "2.0"

#define ERROR_INVALID_REQUEST_MESSAGE "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND_MESSAGE "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL_MESSAGE "Internal error"
#define ERROR_INTERNAL_CODE -32603


static void fail_unless(boolean b, char *s)
{
	if (!b)
		printf("%s\n", s);
}

void test_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	struct json_object *origin = *((struct json_object **)arg);
	fail_unless(json_equals(res, *((struct json_object **)arg)) == 0, "invalid result");
//	char *ress = json_to_string(res);
//	char *reso = json_to_string(origin);
//	if (json_int_get(json_object_get(json_array_get(res, 1), "id")) != 2)
		json_ref_put(res);
	event_loopbreak();
}

struct json_object *create_single_request(struct json_object *method, struct json_object *params, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_PARAMS, params) == 0, "add");
	if (json_type(id) != json_type_null)
		fail_unless(json_object_add(obj, REQUEST_ID, id) == 0, "add");
	else
		json_ref_put(id);
	fail_unless(json_object_add(obj, REQUEST_METHOD, method) == 0, "add");

	return obj;
}


struct json_object *create_single_success_response(struct json_object *result, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_ID, id) == 0, "add");
	fail_unless(json_object_add(obj, RESPONSE_RESULT, result) == 0, "add");

	return obj;
}

struct json_object *create_error(char *message, int code)
{
	struct json_object *j_err = json_object_new();
	fail_unless(j_err != NULL, "object_new");
	fail_unless(json_object_add(j_err, RESPONSE_ERROR_MESSAGE, json_string_new(message)) == 0, "add");
	fail_unless(json_object_add(j_err, RESPONSE_ERROR_CODE, json_int_new(code)) == 0, "add");

	return j_err;
}

struct json_object *create_single_error_response(struct json_object *j_err, struct json_object *id)
{
	struct json_object *obj = json_object_new();
	fail_unless(obj != NULL, "object_new");
	fail_unless(json_object_add(obj, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(obj, REQUEST_ID, id) == 0, "add");
	fail_unless(json_object_add(obj, RESPONSE_ERROR, j_err) == 0, "add");

	return obj;
}

struct json_object *create_batched_request(struct json_object *methods, struct json_object *params, struct json_object *ids)
{
	struct json_object *ar = json_array_new();
	fail_unless(ar != NULL, "array_new");
	int i, len = json_array_length(methods);
	for (i = 0; i < len; i++)
		json_array_add(ar, create_single_request(	json_ref_get(json_array_get(methods, i)),
													json_ref_get(json_array_get(params, i)),
													json_ref_get(json_array_get(ids, i))));



	json_ref_put(methods);
	json_ref_put(params);
	json_ref_put(ids);
	return ar;
}

struct json_object *create_batched_response(struct json_object *results,struct json_object *ids)
{
	struct json_object *ar = json_array_new();
	fail_unless(ar != NULL, "array_new");
	int i, len = json_array_length(results);
	struct json_object *entry;
	for (i = 0; i < len; i++) {
		entry = json_array_get(results, i);
		if (json_object_get(entry, "success"))
			json_array_add(ar, create_single_success_response(json_ref_get(json_object_get(entry, "success")),json_ref_get(json_array_get(ids, i))));
		else
			json_array_add(ar, create_single_error_response(json_ref_get(json_object_get(entry, "error")),json_ref_get(json_array_get(ids, i))));

	}

	json_ref_put(results);
	json_ref_put(ids);
	return ar;
}

void test_1(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_array, "type");
	fail_unless(json_array_length(p) == 1, "length");
	fail_unless(json_type(json_array_get(p, 0)) == json_type_int, "get");
	fail_unless(json_int_get(json_array_get(p, 0)) == 2, "invalid parameter");

	json_rpc_return(jr, p);
}

void test_2(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_object, "type");
	fail_unless(json_type(json_object_get(p, "c")) == json_type_int, "get");
	fail_unless(json_type(json_object_get(p, "d")) == json_type_null, "get");

	fail_unless(json_int_get(json_object_get(p, "c")) == 2, "get");

	json_rpc_return(jr, p);
}

void test_1n(struct json_rpc *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_array, "type");
	fail_unless(json_array_length(p) == 1, "length");
	fail_unless(json_type(json_array_get(p, 0)) == json_type_int, "get");
	fail_unless(json_int_get(json_array_get(p, 0)) == 2, "get");

//	json_ref_put(p);

	json_rpc_return(jr, p);
}


void test_notification(struct json_rpc *jr, struct json_object *res, void *arg)
{
	fail_unless(FALSE, "notification");
}

void v2()
{
	struct json_object *call, *origin;
	struct json_rpc *jr;

	event_init();
	jr = json_rpc_init();
	fail_unless(jr != NULL, "init error");

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_3\", \"test_2\"]"),
									json_parser_parse("[[2], [3], {\"c\":2, \"d\":null}]"),
									json_parser_parse("[1, 2, 3]"));

	struct json_object *err_object = json_object_new();
	fail_unless(json_object_add(err_object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add");

	struct json_object *res_array = json_array_new();

	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":[2]}")) == 0, "add");
	fail_unless(json_array_add(res_array, err_object) == 0, "add");
	fail_unless(json_array_add(res_array, json_parser_parse("{\"success\":{\"c\":2, \"d\":null}}")) == 0, "add");

	origin = create_batched_response(res_array, json_parser_parse("[1, 2, 3]"));

	json_rpc_process(jr, call, test_result, &origin);

	event_dispatch();

//	json_ref_put(call);
	json_ref_put(origin);
	json_rpc_destroy(jr);
}

void v1()
{
	struct json_object *call, *origin;
	struct json_rpc *jr;

	event_init();
	jr = json_rpc_init();

	fail_unless(jr != NULL, "init error");

	fail_unless(json_rpc_add_method(jr, "test_1n", test_1n, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1n\", \"test_1n\", \"test_1n\"]"),
									json_parser_parse("[[2], [2], [2]]"),
									json_parser_parse("[null, null, null]"));

	origin = NULL;

	json_rpc_process(jr, call, test_notification, NULL);

	event_dispatch();

//	json_ref_put(call);
	json_ref_put(origin);
	json_rpc_destroy(jr);
}

void sample_cb(struct evhttp_request *req, void *arg)
{
	fprintf(stdout, "CALLBACK. URI : %s\n", req->uri);
	evhttp_send_reply(req, 200, "OKI-POKI", NULL);
}


int connect_to_port(int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sin_server;
	struct hostent *host_serv;
	memset(&sin_server, '\0', sizeof(sin_server));
	sin_server.sin_family = AF_INET;
	sin_server.sin_port = htons(port);
	host_serv = gethostbyname("localhost");
	memcpy((char *)&sin_server.sin_addr, host_serv->h_addr_list[0], host_serv->h_length);
	if (connect(sock, (struct sockaddr *)&sin_server, sizeof(sin_server)) == -1){
		perror("connect");
		return -1;
	}
	return sock;
}

void read_response(int sock, short type, void *arg)
{
	int ret;
	char buf[250];
	ret = read(sock, buf, 249);
	fail_unless(ret > 0, "read_error");
	buf[ret] = '\0';

	char *content_type_header = strstr(buf, "application/json-rpc");
	fail_unless(content_type_header != NULL, "response doesn`t contain needed header");

	char *json_rpc_resp = strstr(buf, "{");

	struct json_object *res = json_parser_parse(json_rpc_resp);
	struct json_object *origin = (struct json_object *)arg;

	char *res_str = json_to_string(res);
	fail_unless(json_equals(res, origin) == 0, res_str);

	json_ref_put(res);
	free(res_str);

	event_loopexit(NULL);
}

struct evhttp *eh;
struct json_rpc *jr;
struct event *ev_read;

void prepare_server_side()
{
	eh = evhttp_start("127.0.0.1", 7777);
	jr = json_rpc_init();

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");

	evhttp_set_json_rpc(eh, "/json_rpc", jr);
}

void clean_server_side()
{
	evhttp_free(eh);
	json_rpc_destroy(jr);
}

void post_request(int sock, struct json_object *call)
{
	char buf [1024] = {0};
	char *str = json_to_string(call);

	sprintf(buf, "POST /json_rpc HTTP/1.1\r\nContent-Length: %d\r\n\r\n%s", strlen(str), str);

	write(sock, buf, strlen(buf));

	free(str);
}

struct json_object *call, *origin;

void prepare_client_side()
{
	int port = 7777;
	int sock = connect_to_port(port);

	call = create_single_request(json_string_new("test_1"), json_parser_parse("[2]"), json_int_new(1));

	post_request(sock, call);

	origin = create_single_success_response(json_parser_parse("[2]"), json_int_new(1));

	ev_read = (struct event *)malloc(sizeof(struct event));
	fail_unless(ev_read != NULL, "malloc");
	event_set(ev_read, sock, EV_READ, read_response, (void *)origin);
	fail_unless(event_add(ev_read, NULL) == 0, "event_add");
}

void clean_client_side()
{
	json_ref_put(call);
	json_ref_put(origin);
	event_del(ev_read);
	free(ev_read);
}
int main()
{
//	event_init();
//
//	prepare_server_side();
//	prepare_client_side();
//
//	event_dispatch();
//
//	clean_server_side();
//	clean_client_side();

	v2();

	return 0;
}

//	eh = evhttps_start("127.0.0.1", 8888, "serv.pem", "serv.pem", "1234");
