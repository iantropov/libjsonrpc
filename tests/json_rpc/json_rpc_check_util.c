#include <event.h>

#include "../../src/json_parser.h"
#include <check.h>

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

#define RESPONSE_ERROR "error"
#define RESPONSE_ERROR_MESSAGE "message"
#define RESPONSE_ERROR_CODE "code"
#define RESPONSE_RESULT "result"

#define JSON_RPC_VERSION "2.0"

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
