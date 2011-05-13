#include "check_json_rpc_over_http.h"

#include <event.h>

#include "../../src/json_parser.h"
#include "json_rpc_check_util.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#define REQUEST_ID "id"
#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"
#define REQUEST_VERSION "jsonrpc"

#define JSON_RPC_VERSION "2.0"

#define ERROR_INVALID_REQUEST_MESSAGE "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND_MESSAGE "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL_MESSAGE "Internal error"
#define ERROR_INTERNAL_CODE -32603

#define HTTP_PORT 7777

struct json_rpc *jr;
struct json_object *origin, *call;
struct evhttp *eh;
struct event *ev_read;
int sock;
const struct timeval tv = {0, 100000};

static void test_1(struct json_rpc_request *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_array, "type");
	fail_unless(json_array_length(p) == 1, "length");
	fail_unless(json_type(json_array_get(p, 0)) == json_type_int, "get");
	fail_unless(json_int_get(json_array_get(p, 0)) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
}

static void test_2(struct json_rpc_request *jr, struct json_object *p, void *arg)
{
	fail_unless(json_type(p) == json_type_object, "type");
	fail_unless(json_type(json_object_get(p, "c")) == json_type_int, "get");
	fail_unless(json_type(json_object_get(p, "d")) == json_type_null, "get");

	fail_unless(json_int_get(json_object_get(p, "c")) == 2, "get");

	json_rpc_return(jr, json_ref_get(p));
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

static void read_response(int sock, short type, void *arg)
{
	int ret;
	char buf[250];

	ret = read(sock, buf, 249);
	fail_unless(ret > 0, "read_error");
	buf[ret] = '\0';

	char *content_type_header = strstr(buf, "application/json-rpc");
	fail_unless(content_type_header != NULL, "response doesn`t contain needed header");

	char *p1 = strchr(buf, '[');
	char *p2 = strchr(buf, '{');

	char *json_rpc_resp = p1 < p2 && p1 != NULL ? p1 : p2;

	struct json_object *res = json_parser_parse(json_rpc_resp);

	fail_unless(json_equals(res, origin) == 0, "not equals");

	event_loopexit(NULL);
}

static void prepare_server_side()
{
	eh = evhttp_start("127.0.0.1", HTTP_PORT);
	jr = json_rpc_new();

	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	evhttp_set_json_rpc(eh, "/json_rpc", jr);
}

static void clean_server_side()
{
	evhttp_free(eh);
	json_rpc_free(jr);
}

static void post_request(int sock, struct json_object *call)
{
	char buf [1024] = {0};
	char *str = json_to_string(call);

	sprintf(buf, "POST /json_rpc HTTP/1.1\r\nContent-Length: %d\r\n\r\n%s", strlen(str), str);

	write(sock, buf, strlen(buf));

	free(str);
}

static void prepare_client_side()
{
	sock = connect_to_port(HTTP_PORT);

	ev_read = (struct event *)malloc(sizeof(struct event));
	fail_unless(ev_read != NULL, "malloc");
	event_set(ev_read, sock, EV_READ, read_response, (void *)origin);
	fail_unless(event_add(ev_read, NULL) == 0, "event_add");
}

static void clean_client_side()
{
	event_del(ev_read);
	free(ev_read);
	close(sock);
}

static void setup()
{
	event_init();

	prepare_server_side();
	prepare_client_side();
}

static void teardown()
{
	clean_server_side();
	clean_client_side();
}

static void exit_loop(int s, short t, void *arg)
{
	event_loopexit(NULL);
}


START_TEST (test_single_success_1)
{
	call = create_single_request(json_string_new("test_1"), json_parser_parse("[2]"), json_int_new(1));

	origin = create_single_success_response(json_parser_parse("[2]"), json_int_new(1));

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_single_error_1)
{
	call = create_single_request(json_string_new("test_3"), json_parser_parse("[]"), json_string_new("id"));

	origin = create_single_error_response(create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE), json_string_new("id"));

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_single_error_2)
{
	call = json_object_new();
	fail_unless(call != NULL, "object_new");
	fail_unless(json_object_add(call, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");
	fail_unless(json_object_add(call, REQUEST_ID, json_string_new("id")) == 0, "add");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	post_request(sock, call);

	event_dispatch();
}

END_TEST

START_TEST (test_batched_success_1)
{
	call = create_batched_request(	json_parser_parse("[\"test_1\"]"),
									json_parser_parse("[[2]]"),
									json_parser_parse("[1]"));

	origin = create_batched_response(	json_parser_parse("[{\"success\":[2]}]"),
										json_parser_parse("[1]"));

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_1)
{
	call = create_batched_request(	json_parser_parse("[\"test_3\"]"),
									json_parser_parse("[[2]]"),
									json_parser_parse("[1]"));

	struct json_object *array = json_array_new();
	struct json_object *object = json_object_new();
	fail_unless(json_object_add(object, "error", create_error(ERROR_METHOD_NOT_FOUND_MESSAGE, ERROR_METHOD_NOT_FOUND_CODE)) == 0, "add");
	fail_unless(json_array_add(array, object) == 0, "add");
	origin = create_batched_response(array, json_parser_parse("[1]"));

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_error_2)
{
	call = json_array_new();
	fail_unless(call != NULL, "array_new");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_notification_1)
{
	call = create_single_request(json_string_new("test_1"), json_parser_parse("[2]"), json_null_new());

	origin = NULL;

	post_request(sock, call);

	event_once(-1, EV_TIMEOUT, exit_loop, NULL, &tv);

	event_dispatch();
}
END_TEST

START_TEST (test_notification_2)
{
	call = json_object_new();
	fail_unless(call != NULL, "object_new");
	fail_unless(json_object_add(call, REQUEST_VERSION, json_string_new(JSON_RPC_VERSION)) == 0, "add");

	origin = create_single_error_response(create_error(ERROR_INVALID_REQUEST_MESSAGE, ERROR_INVALID_REQUEST_CODE), json_null_new());

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_notification_1)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\", \"test_2\"]"),
									json_parser_parse("[[2], {\"c\":2, \"d\":null}, {\"c\":2, \"d\":null}]"),
									json_parser_parse("[1, null, \"cdcd\"]"));

	origin = create_batched_response(	json_parser_parse("[{\"success\":[2]}, {\"success\":{\"c\":2, \"d\":null}}]"),
										json_parser_parse("[1, \"cdcd\"]"));

	post_request(sock, call);

	event_dispatch();
}
END_TEST

START_TEST (test_batched_notification_2)
{
	fail_unless(json_rpc_add_method(jr, "test_1", test_1, NULL) == 0, "add");
	fail_unless(json_rpc_add_method(jr, "test_2", test_2, NULL) == 0, "add");

	call = create_batched_request(	json_parser_parse("[\"test_1\", \"test_2\", \"test_2\"]"),
									json_parser_parse("[[2], {\"c\":2, \"d\":null}, {\"c\":2, \"d\":null}]"),
									json_parser_parse("[null, null, null]"));

	origin = NULL;

	post_request(sock, call);

	event_once(-1, EV_TIMEOUT, exit_loop, NULL, &tv);

	event_dispatch();
}
END_TEST


TCase *json_rpc_over_http_tcase(void)
{
	/* Json object creation test case */
	TCase *tc = tcase_create ("json_rpc_process");
	tcase_add_checked_fixture(tc, setup, teardown);

	tcase_add_test (tc, test_single_success_1);

	tcase_add_test (tc, test_single_error_1);
	tcase_add_test (tc, test_single_error_2);

	tcase_add_test (tc, test_batched_success_1);

	tcase_add_test (tc, test_batched_error_1);
	tcase_add_test (tc, test_batched_error_2);

	tcase_add_test(tc, test_notification_1);
	tcase_add_test(tc, test_notification_2);

	tcase_add_test(tc, test_batched_notification_1);
	tcase_add_test(tc, test_batched_notification_2);

	return tc;
}
