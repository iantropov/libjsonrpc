/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include <stdio.h>
#include "json_parser.h"
#include "json_rpc.h"
#include "json_rpc_tt.h"

static void fail_unless(int a, char *str)
{
	if (!a)
		fprintf(stderr, "%s\n", str);
}

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <event0/bufevent.h>
#include <event.h>

#include "json_rpc.h"
#include "json_parser.h"
#include "json_rpc_tt.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

int util_connect_to_port(int port)
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

void util_send_handshake(struct bufevent *bufev, char *uri, char *host, int port)
{
	struct evbuffer *buf = evbuffer_new();

	evbuffer_add_printf(buf, "GET %s HTTP/1.1\r\n", uri);
	evbuffer_add_printf(buf, "Upgrade: WebSocket\r\n");
	evbuffer_add_printf(buf, "Connection: Upgrade\r\n");
	evbuffer_add_printf(buf, "Host: %s:%d\r\n", host, port);
	evbuffer_add_printf(buf, "Origin: null\r\n");

	evbuffer_add_printf(buf, "Sec-WebSocket-Key1: 18x 6]8vM;54 *(5:  {   U1]8  z [  8\r\n");
	evbuffer_add_printf(buf, "Sec-WebSocket-Key2: 1_ tx7X d  <  nw  334J702) 7]o}` 0\r\n");

	evbuffer_add_printf(buf, "\r\n");

	evbuffer_add(buf, "Tm[K T2u", 8);

	bufevent_write_buffer(bufev, buf);

	evbuffer_free(buf);
}

void util_send_ws_closing_frame(struct bufevent *bufev)
{
	struct evbuffer *buf = evbuffer_new();

	evbuffer_add(buf, "\xff", 1);

	evbuffer_add(buf, "\x00", 1);

	bufevent_write_buffer(bufev, buf);

	evbuffer_free(buf);
}

void util_send_ws_frame(struct bufevent *bufev, u_char *mess)
{
	struct evbuffer *buf = evbuffer_new();

	evbuffer_add(buf, "\x00", 1);

	evbuffer_add(buf, mess, strlen((char *)mess));

	evbuffer_add(buf, "\xff", 1);

	bufevent_write_buffer(bufev, buf);

	evbuffer_free(buf);
}

struct json_object *util_get_json_from_ws_frame(struct bufevent *bufev)
{
	struct evbuffer *buf = bufevent_get_input(bufev);

	char *str = strndup(buf->buffer +1, buf->off - 2);

	struct json_object *obj = json_parser_parse(str);

	free(str);

	return obj;
}

#define CORRECT_REQUEST "{\"jsonrpc\":\"2.0\", \"method\":\"test1\", \"params\":[2], \"id\":2}"
#define CORRECT_PARAMS "[2]"
#define CORRECT_RESPONSE "{\"jsonrpc\":\"2.0\", \"result\":[2], \"id\":2}"
#define INCORRECT_REQUEST "{\"jsonrpc\":\"2.0\", \"method\":\"test8\", \"params\":[2], \"id\":2}"
#define INCORRECT_RESPONSE "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":-32601, \"message\":\"Method not found\"}, \"id\": 2}"

#define WS_URI "/"
#define HTTP_PORT 7777
#define HTTP_HOST "127.0.0.1"

#define CB_ARG_VALUE 0xdffd
#define MESSAGE "Hello"

static struct evhttp *__eh;
static struct bufevent *__client_bufev;
static int __client_sock;
static const struct timeval __tv = {0, 1};

static void (*__commands[100])();
static int __command_counter = 0;
static int __waiting_command_counter;
static u_char *__message;
static short __what;
static struct ws_connection *__ws_conn;
static struct ws_accepter *__ws_accepter;

static struct json_rpc_tt *__server_tt;

struct json_rpc_request *__jrpc_req;
struct json_object *__input_obj;
struct json_object *__output_obj;

static struct json_rpc *__server_jr;
static struct json_rpc *__client_jr;

static int process_commands(void(**commands)(), int command_counter)
{
	void(*counter)();
	for (counter = commands[command_counter++];counter != NULL; counter = commands[command_counter++])
		counter();

	return command_counter;
}

static void start_process_commands()
{
	__command_counter = process_commands(__commands, __command_counter);
	event_dispatch();
}

static void add_command(void (*cb)())
{
	__commands[__waiting_command_counter++] = cb;
}

static void check_arg(void *arg)
{
	fail_unless(arg == (void *)CB_ARG_VALUE, "error cb_arg");
}

static void test_cb(struct json_rpc_request *req, struct json_object *obj, void *arg)
{
	__jrpc_req = req;
	__input_obj = obj;

	__command_counter = process_commands(__commands, __command_counter);
}

static void ws_errorcb(struct ws_connection *conn, short what, void *arg)
{
	check_arg(arg);

	__what = what;
	__ws_conn = conn;
	__command_counter = process_commands(__commands, __command_counter);
}

static void client_readcb(struct bufevent *bufev, void *arg)
{
	check_arg(arg);

	__command_counter = process_commands(__commands, __command_counter);

	struct evbuffer *buf = bufevent_get_input(bufev);
	evbuffer_drain(buf, buf->off);
}

static void client_errorcb(struct bufevent *bufev, short what, void *arg)
{
	check_arg(arg);

	__what = what;
	__command_counter = process_commands(__commands, __command_counter);
}

static void jrpc_result_cb_client(struct json_rpc *jr, struct json_object *obj, void *arg)
{
	char *str = json_to_string(obj);

	util_send_ws_frame(__client_bufev, str);

	free(str);
	json_ref_put(obj);
}

static void result_cb(struct json_rpc *jr, struct json_object *obj, void *arg)
{
	check_arg(arg);

	__output_obj = obj;
	__command_counter = process_commands(__commands, __command_counter);
}

static void ws_acceptcb(struct ws_accepter *wa, struct bufevent *bufev, void *arg)
{
	check_arg(arg);

	__ws_conn = ws_connection_new(bufev, NULL, ws_errorcb, (void *)CB_ARG_VALUE);
	fail_unless(__ws_conn != NULL, "ws_new failed");

	__server_tt = json_rpc_tt_ws_new(__server_jr, __ws_conn, ws_errorcb, (void *)CB_ARG_VALUE);
	fail_unless(__server_tt != NULL, "ws_json_rpc_failed");

//	__command_counter = process_commands(__commands, __command_counter);
}

static void prepare_server_side()
{
	__eh = evhttp_start(HTTP_HOST, HTTP_PORT);

	__server_jr = json_rpc_new();
	fail_unless(__server_jr != NULL, "json_rpc_new failed");

	__ws_accepter = ws_accepter_new(__eh, WS_URI, ws_acceptcb, (void *)CB_ARG_VALUE);
}

static void clean_server_side()
{
	ws_accepter_free(__ws_accepter);
	evhttp_free(__eh);
	json_rpc_tt_free(__server_tt);
	json_rpc_free(__server_jr);
}

static void prepare_client_side()
{
	__client_jr = json_rpc_new();
	fail_unless(__client_jr != NULL, "json_rpc_new failed");

	__client_sock = util_connect_to_port(HTTP_PORT);

	__client_bufev = bufevent_new(__client_sock, client_readcb, NULL, client_errorcb, (void *)CB_ARG_VALUE);
	fail_unless(__client_bufev != NULL, "bufevent_new");
	bufevent_enable(__client_bufev, EV_READ);
}

static void clean_client_side()
{
	bufevent_free(__client_bufev);
	close(__client_sock);
	json_rpc_free(__client_jr);
}

static void setup()
{
	event_init();

	__waiting_command_counter = __command_counter = 0;

	prepare_server_side();
	prepare_client_side();
}

static void teardown()
{
	fail_unless(__waiting_command_counter  + 1 == __command_counter, "Not all cbs are executed");

	clean_server_side();
	clean_client_side();
}

static void loop_exit()
{
	event_loopexit(NULL);
}

static void send_handshake_client()
{
	util_send_handshake(__client_bufev, WS_URI, HTTP_HOST, HTTP_PORT);
}

static void send_closing_frame_server()
{
	ws_connection_send_close(__ws_conn);
}

static void send_closing_frame_client()
{
	util_send_ws_closing_frame(__client_bufev);
}

static void send_request(struct json_rpc_tt *wj, char *str)
{
	struct json_object *obj = json_parser_parse(str);
	fail_unless(obj != NULL, "json_parser_parse");

	json_rpc_tt_send(wj, obj, result_cb, (void *)CB_ARG_VALUE);

	json_ref_put(obj);
}

static void send_correct_request_server()
{
	send_request(__server_tt, CORRECT_REQUEST);
}

static void send_incorrect_request_server()
{
	send_request(__server_tt, INCORRECT_REQUEST);
}

static void send_correct_request_client()
{
	util_send_ws_frame(__client_bufev, (u_char *)CORRECT_REQUEST);
}

static void send_incorrect_request_client()
{
	util_send_ws_frame(__client_bufev, (u_char *)INCORRECT_REQUEST);
}

static void test_correct_request_server()
{
	fail_unless(__jrpc_req != NULL, "incorrect json_rpc_request");

	struct json_object *obj = json_parser_parse(CORRECT_PARAMS);
	fail_unless(obj != NULL, "parser parse");

	fail_unless(json_equals(obj, __input_obj) == 0, "incorrect request");

	json_ref_put(obj);
	json_ref_put(__input_obj);
}

static void test_correct_request_client()
{
	test_correct_request_server();
}

static void handle_request_client()
{
	json_rpc_process_request(__client_jr, util_get_json_from_ws_frame(__client_bufev), jrpc_result_cb_client, NULL);
}

static void send_correct_response_server()
{
	struct json_object *obj = json_parser_parse(CORRECT_PARAMS);
	fail_unless(obj != NULL, "parser parse");

	json_rpc_return(__jrpc_req, obj);
}

static void send_correct_response_client()
{
	send_correct_response_server();
}

static void test_response_server(char *response)
{
	struct json_object *obj = json_parser_parse(response);
	fail_unless(obj != NULL, "parser parse");

	fail_unless(json_equals(obj, __output_obj) == 0, "incorrect response");

	json_ref_put(obj);
	json_ref_put(__output_obj);
}

static void test_correct_response_server()
{
	test_response_server(CORRECT_RESPONSE);
}

static void test_incorrect_response_server()
{
	test_response_server(INCORRECT_RESPONSE);
}

static void test_response_client(char *response)
{
	struct json_object *obj_exp = json_parser_parse(response);
	fail_unless(obj_exp != NULL, "parser parse");

	struct json_object *obj_was = util_get_json_from_ws_frame(__client_bufev);
	fail_unless(obj_was != NULL, "parser parse");

	fail_unless(json_equals(obj_exp, obj_was) == 0, "incorrect response");

	json_ref_put(obj_exp);
	json_ref_put(obj_was);
}

static void test_correct_response_client()
{
	test_response_client(CORRECT_RESPONSE);
}

static void test_incorrect_response_client()
{
	test_response_client(INCORRECT_RESPONSE);
}

static void test_ws_error_server()
{
	fail_unless(__what == WS_CLOSING_FRAME, "incorrect error reason");
}

static void add_jrpc_method(struct json_rpc *jr)
{
	json_rpc_add_method(jr, "test1", test_cb, NULL);
}

static void add_jrpc_method_server()
{
	add_jrpc_method(__server_jr);
}

static void add_jrpc_method_client()
{
	add_jrpc_method(__client_jr);
}


void test()
{
	add_command(send_handshake_client);
	add_command(NULL);
	add_command(send_closing_frame_client);
	add_command(NULL);
	add_command(test_ws_error_server);
	add_command(loop_exit);

	start_process_commands();
}

int main()
{
	setup();
	test();
	teardown();
}

//	eh = evhttps_start("127.0.0.1", 8888, "serv.pem", "serv.pem", "1234");
