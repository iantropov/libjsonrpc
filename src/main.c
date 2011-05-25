/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "json_rpc.h"
#include "json_parser.h"

static void fail_unless(int a, char *str)
{
	if (!a)
		fprintf(stderr, "%s\n", str);
}

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#define CORRECT_REQUEST "{\"jsonrpc\":\"2.0\", \"method\":\"test1\", \"params\":[2], \"id\":2}"
#define CORRECT_PARAMS "[2]"
#define CORRECT_RESPONSE "{\"jsonrpc\":\"2.0\", \"result\":[2], \"id\":2}"
#define INCORRECT_REQUEST "{\"jsonrpc\":\"2.0\", \"meod\":\"test1\", \"params\":[2], \"id\":2}"
#define INCORRECT_RESPONSE "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":-32600, \"message\":\"Invalid request\"}, \"id\": null}"

static int __port;

static struct bufevent *__server_bufev;
static struct bufevent *__client_bufev;

static struct json_rpc *__server_jr;
static struct json_rpc *__client_jr;
static struct json_rpc *__jr;

static int __accept_sock;

static struct json_object *__input_obj;
static struct json_object *__output_obj;

static struct json_rpc_request *__jrpc_req;

static struct bufevent_jrpc *__client_bjrpc;
static struct bufevent_jrpc *__server_bjrpc;

static void (*__commands[100])();
static int __command_counter = 0;
static short __what;

static int get_random_port(int low, int high)
{
	time_t seed;
	time(&seed);
	if (low >= high)
		return -1;
	srandom(seed);
	int n = random();
	int res = low + n % (high - low);
	return res;
}

static int connect_to_port(int port)
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

static int prepare_server_socket(int port)
{
	int sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_server == -1) {
		return -1;
	}
	struct sockaddr_in sin_server;
	memset(&sin_server, '\0', sizeof(struct sockaddr_in));
	sin_server.sin_family = AF_INET;
	sin_server.sin_port = htons(port);/*FIXME*/
	sin_server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock_server, (struct sockaddr *)&sin_server,
							sizeof(struct sockaddr_in)) == -1) {
		return -1;
	}
	if (listen(sock_server, 10) == -1) {
		return -1;
	}
	if (fcntl(sock_server, F_SETFL, O_NONBLOCK) == -1) {
		return -1;
	}
	return sock_server;
}

static int accept_connection(int server_sock)
{
	struct sockaddr_in sin_client;
	int from_len = sizeof(sin_client);
	int sock_client = accept(server_sock, (struct sockaddr *)&sin_client, (socklen_t *)&from_len);

	return sock_client;
}

static int process_commands(void(**commands)(), int command_counter)
{
	void(*counter)();
	for (counter = commands[command_counter++];counter != NULL; counter = commands[command_counter++])
		counter();

	return command_counter;
}

static void start_process_commands()
{
	__command_counter = 0;
	__command_counter = process_commands(__commands, __command_counter);
	event_dispatch();
}

static void add_command(void (*cb)())
{
	__commands[__command_counter++] = cb;
}

static void test_cb(struct json_rpc_request *req, struct json_object *obj, void *arg)
{
	__jrpc_req = req;
	__input_obj = obj;

//	fail_unless(7 == 8, "cdcd");

	__command_counter = process_commands(__commands, __command_counter);
}

static void result_cb(struct json_rpc *jr, struct json_object *obj, void *arg)
{
	__output_obj = obj;
	__jr = jr;

//	fail_unless(7 == 8, "cdcd");

	__command_counter = process_commands(__commands, __command_counter);
}

static void client_error_cb(struct bufevent *bufev, short what, void *arg)
{
	__what = what;
	__command_counter = process_commands(__commands, __command_counter);
}

static void server_error_cb(struct bufevent *bufev, short what, void *arg)
{
	__what = what;
	__command_counter = process_commands(__commands, __command_counter);
}

static void clean_server_side()
{
	if (__server_bjrpc != NULL)
		bufevent_json_rpc_free(__server_bjrpc);
	json_rpc_free(__server_jr);
}

static void prepare_server_side()
{
	int server_sock = accept_connection(__accept_sock);
	fail_unless(server_sock > 0, "accept_connection");
	__server_bufev = bufevent_new(server_sock, NULL, NULL, NULL, NULL);
	__server_jr = json_rpc_new();
	__server_bjrpc = bufevent_json_rpc_new(__server_bufev, __server_jr, server_error_cb, NULL);
	bufevent_enable(__server_bufev, EV_READ);
}

static void prepare_client_side()
{
	int client_sock = connect_to_port(__port);
	fail_unless(client_sock > 0, "connect_to_port");
	__client_bufev = bufevent_new(client_sock, NULL, NULL, NULL, NULL);
	__client_jr = json_rpc_new();
	__client_bjrpc = bufevent_json_rpc_new(__client_bufev, __client_jr, client_error_cb, NULL);
	bufevent_enable(__client_bufev, EV_READ);
}

static void clean_client_side()
{
	bufevent_json_rpc_free(__client_bjrpc);
	json_rpc_free(__client_jr);
}

static void setup()
{
	event_init();

	__port = get_random_port(7000, 10000);
	__accept_sock = prepare_server_socket(__port);
	fail_unless(__accept_sock > 0, "prepare_server_socket");
	prepare_client_side();
	prepare_server_side();
}

static void teardown()
{
	clean_server_side();
	clean_client_side();

	close(__accept_sock);

	sleep(1);
}

static void loop_exit()
{
	event_loopbreak();
}

static void send_correct_request(struct bufevent_jrpc *bj)
{
	struct json_object *obj = json_parser_parse(CORRECT_REQUEST);
	fail_unless(obj != NULL, "json_parser_parse");

	bufevent_json_rpc_send(bj, obj, result_cb, NULL);

	json_ref_put(obj);
}

static void send_correct_request_server()
{
	send_correct_request(__server_bjrpc);
}

static void send_correct_request_client()
{
	send_correct_request(__client_bjrpc);
}

static void send_incorrect_request(struct bufevent *bufev)
{
	bufevent_write(bufev, INCORRECT_REQUEST, strlen(INCORRECT_REQUEST));
}

static void send_incorrect_request_server()
{
	send_incorrect_request(__server_bufev);
}

static void send_incorrect_request_client()
{
	send_incorrect_request(__client_bufev);
}

static void test_correct_request()
{
	fail_unless(__jrpc_req != NULL, "incorrect json_rpc_request");

	struct json_object *obj = json_parser_parse(CORRECT_PARAMS);
	fail_unless(obj != NULL, "parser parse");

	fail_unless(json_equals(obj, __input_obj) == 0, "incorrect request");

	json_ref_put(obj);
	json_ref_put(__input_obj);
}

static void send_correct_response()
{
	struct json_object *obj = json_parser_parse(CORRECT_PARAMS);
	fail_unless(obj != NULL, "parser parse");

	json_rpc_return(__jrpc_req, obj);
}

static void test_response(char *response)
{
	struct json_object *obj = json_parser_parse(response);
	fail_unless(obj != NULL, "parser parse");

	fail_unless(json_equals(obj, __output_obj) == 0, "incorrect response");

	json_ref_put(obj);
	json_ref_put(__output_obj);
}

static void test_correct_response()
{
	test_response(CORRECT_RESPONSE);
}

static void test_incorrect_response()
{
	test_response(INCORRECT_RESPONSE);
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

static void close_socket_server()
{
	bufevent_json_rpc_free(__server_bjrpc);
	__server_bjrpc = NULL;
}

static void test_bufevent_error()
{
	fail_unless((__what & EVBUFFER_EOF) != 0, "bufevent error reason");
}

void test()
{
	add_command(add_jrpc_method_server);
	add_command(send_incorrect_request_client);
//	add_command(test_incorrect_response);
	add_command(send_correct_request_client);
	add_command(NULL);
	add_command(test_correct_request);
	add_command(send_correct_response);
	add_command(NULL);
	add_command(test_correct_response);
	add_command(loop_exit);

	start_process_commands();
}

int main()
{
	setup();
	test();
	teardown();

	return 0;
}

//	eh = evhttps_start("127.0.0.1", 8888, "serv.pem", "serv.pem", "1234");
