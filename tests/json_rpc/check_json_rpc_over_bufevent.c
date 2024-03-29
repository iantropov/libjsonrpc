#include "check.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <event0/bufevent.h>
#include <event.h>

#include "../../src/json_rpc.h"
#include "../../src/json_rpc_tt.h"
#include "../../src/json_parser.h"

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
#define INCORRECT_REQUEST "{\"jsonrpc\":\"2.0\", \"method\":\"test8\", \"params\":[2], \"id\":2}"
#define INCORRECT_RESPONSE "{\"jsonrpc\":\"2.0\", \"error\":{\"code\":-32601, \"message\":\"Method not found\"}, \"id\": 2}"

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

static struct json_rpc_tt *__client_tt;
static struct json_rpc_tt *__server_tt;

static void (*__commands[100])();
static int __command_counter = 0;
static int __waiting_command_counter;
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
	__commands[__waiting_command_counter++] = cb;
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
	if (__server_tt != NULL)
		json_rpc_tt_free(__server_tt);
	json_rpc_free(__server_jr);
}

static void prepare_server_side()
{
	int server_sock = accept_connection(__accept_sock);
	fail_unless(server_sock > 0, "accept_connection");
	__server_bufev = bufevent_new(server_sock, NULL, NULL, NULL, NULL);
	__server_jr = json_rpc_new();
	__server_tt = json_rpc_tt_tcp_new(__server_jr, __server_bufev, server_error_cb, NULL);
	bufevent_enable(__server_bufev, EV_READ);
}

static void prepare_client_side()
{
	int client_sock = connect_to_port(__port);
	fail_unless(client_sock > 0, "connect_to_port");
	__client_bufev = bufevent_new(client_sock, NULL, NULL, NULL, NULL);
	__client_jr = json_rpc_new();
	__client_tt = json_rpc_tt_tcp_new(__client_jr, __client_bufev, client_error_cb, NULL);
	bufevent_enable(__client_bufev, EV_READ);
}

static void clean_client_side()
{
	json_rpc_tt_free(__client_tt);
	json_rpc_free(__client_jr);
}

static void setup()
{
	event_init();

	__waiting_command_counter = __command_counter = 0;

	__port = get_random_port(7000, 10000);
	__accept_sock = prepare_server_socket(__port);
	fail_unless(__accept_sock > 0, "prepare_server_socket");
	prepare_client_side();
	prepare_server_side();
}

static void teardown()
{
	fail_unless(__waiting_command_counter  + 1 == __command_counter, "Not all cbs are executed");

	clean_server_side();
	clean_client_side();

	close(__accept_sock);

	sleep(1);
}

static void loop_exit()
{
	event_loopbreak();
}

static void send_json(struct json_rpc_tt *bj, char *str)
{
	struct json_object *obj = json_parser_parse(str);
	fail_unless(obj != NULL, "json_parser_parse");

	json_rpc_tt_send(bj, obj, result_cb, NULL);

	json_ref_put(obj);
}

static void send_correct_request_server()
{
	send_json(__server_tt, CORRECT_REQUEST);
}

static void send_correct_request_client()
{
	send_json(__client_tt, CORRECT_REQUEST);
}

static void send_incorrect_request(struct bufevent *bufev)
{
//	send_json(bufev, INCORRECT_REQUEST, strlen(INCORRECT_REQUEST));
}

static void send_incorrect_request_server()
{
	send_json(__server_tt, INCORRECT_REQUEST);
}

static void send_incorrect_request_client()
{
	send_json(__client_tt, INCORRECT_REQUEST);
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
	json_rpc_tt_free(__server_tt);
	__server_tt = NULL;
}

static void test_bufevent_error()
{
	fail_unless((__what & EVBUFFER_EOF) != 0, "bufevent error reason");
}

START_TEST(test_success_0)
{
	add_command(add_jrpc_method_client);
	add_command(send_correct_request_server);
	add_command(NULL);
	add_command(test_correct_request);
	add_command(send_correct_response);
	add_command(NULL);
	add_command(test_correct_response);
	add_command(loop_exit);

	start_process_commands();
}
END_TEST

START_TEST(test_success_1)
{
	add_command(add_jrpc_method_server);
	add_command(send_incorrect_request_client);
	add_command(NULL);
	add_command(test_incorrect_response);
	add_command(send_correct_request_client);
	add_command(NULL);
	add_command(test_correct_request);
	add_command(send_correct_response);
	add_command(NULL);
	add_command(test_correct_response);
	add_command(loop_exit);

	start_process_commands();
}
END_TEST

START_TEST(test_error_0)
{
	add_command(close_socket_server);
	add_command(NULL);
	add_command(test_bufevent_error);
	add_command(loop_exit);

	start_process_commands();
}
END_TEST

TCase *json_rpc_over_bufevent_tcase()
{
	TCase *tc = tcase_create ("json_rpc_over_bufevent");
	tcase_add_checked_fixture(tc, setup, teardown);

	tcase_add_test(tc, test_success_0);
	tcase_add_test(tc, test_success_1);

	tcase_add_test(tc, test_error_0);

	return tc;
}
