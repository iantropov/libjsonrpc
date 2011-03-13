/*
 * json_rpc.c
 *
 *  Created on: Mar 4, 2011
 *      Author: ant
 */

#include "json_rpc.h"

#include "../util/list.h"
#include "../util/string_functions.h"

#include <event.h>
#include <string.h>

struct json_rpc {
	struct list_head methods;

	json_rpc_method *user_result_method;
	void *user_result_method_arg;

	struct json_object *current_method_id;
	json_rpc_method *process_method;
	void *process_method_arg;

	int batch_length;
	int current_batch_ind;
	struct json_object *batch;
	struct json_object *result_batch;
};

enum call_type {request_call, notification_call, invalid_call};

struct json_rpc_list_entry {
	struct list_head list;

	json_rpc_method *method;
	char *method_name;
	void *method_arg;
};

struct deferred_call_data {
	json_rpc_method *method;
	struct json_object *obj;
	struct json_rpc *jr;
	void *method_arg;

	struct event defer_ev;
} call_data;

const struct timeval defer_tv = {0, 0};

#define REQUEST_ID "id"
#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"
#define REQUEST_VERSION "jsonrpc"

#define RESPONSE_ERROR "error"
#define RESPONSE_ERROR_MESSAGE "message"
#define RESPONSE_ERROR_CODE "code"
#define RESPONSE_RESULT "result"

#define JSON_RPC_VERSION "2.0"

#define ERROR_INVALID_REQUEST "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL "Internal error"
#define ERROR_INTERNAL_CODE -32603


struct json_rpc *json_rpc_init()
{
	struct json_rpc *jr = (struct json_rpc *)malloc(sizeof(struct json_rpc));
	if (jr == NULL)
		return NULL;

	INIT_LIST_HEAD(&jr->methods);

	return jr;
}

void json_rpc_destroy(struct json_rpc *jr)
{
	if (jr == NULL)
		return;

	struct list_head *p, *n;
	struct json_rpc_list_entry *entry;
	list_for_each_safe(p, n, &jr->methods) {
		entry = list_entry(p, struct json_rpc_list_entry, list);
		free(entry->method_name);
		list_del(p);
		free(entry);
	}

	free(jr);
}

int json_rpc_add_method(struct json_rpc *jr, char *name, json_rpc_method *method, void *arg)
{
	struct json_rpc_list_entry *entry = (struct json_rpc_list_entry *)malloc(sizeof(struct json_rpc_list_entry));
	if (entry == NULL)
		return -1;

	entry->method = method;
	entry->method_arg = arg;
	entry->method_name = string_copy(name);
	if (entry->method_name == NULL) {
		free(entry);
		return -1;
	}
	list_add(&entry->list, &jr->methods);

	return 0;
}

void json_rpc_del_method(struct json_rpc *jr, char *name)
{
	if (jr == NULL)
		return;

	struct list_head *p, *n;
	struct json_rpc_list_entry *entry;
	list_for_each_safe(p, n, &jr->methods) {
		entry = list_entry(p, struct json_rpc_list_entry, list);
		if (strcmp(entry->method_name, name))
			continue;
		free(entry->method_name);
		list_del(p);
		free(entry);
	}

	free(jr);
}

static void defer_call_wrap(int s, short t, void *arg)
{
	struct deferred_call_data *data = (struct deferred_call_data *)arg;

	data->method(data->jr, data->obj, data->method_arg);

	free(data);
}

static void add_to_queue(json_rpc_method *method, struct json_rpc *jr, struct json_object *obj, void *arg)
{
	struct deferred_call_data *call_data = (struct deferred_call_data *)malloc(sizeof(struct deferred_call_data));
	if (call_data == NULL)
		return;
	call_data->jr = jr;
	call_data->obj = obj;
	call_data->method = method;
	call_data->method_arg = arg;
	event_set(&call_data->defer_ev, 0, EV_TIMEOUT, defer_call_wrap, (void *)call_data);
	event_add(&call_data->defer_ev, &defer_tv);
}

static struct json_object *prepare_response_object(struct json_object *id, struct json_object *res)
{
	struct json_object *j_res = json_object_new();
	if (j_res == NULL)
		goto error;

	struct json_object *j_ver = json_string_new(JSON_RPC_VERSION);
	if (j_ver == NULL)
		goto error;

	int ret = json_object_add(j_res, REQUEST_VERSION, j_ver);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_res, RESPONSE_RESULT, res);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_res, REQUEST_ID, id);
	if (ret == -1)
		goto error;

	return j_res;

error:
	json_ref_put(j_res);
	return NULL;
}

static struct json_object *create_error_object(char *message, int code)
{
	struct json_object *err = json_object_new();
	if (err == NULL)
		return NULL;

	int ret = json_object_add(err, RESPONSE_ERROR_MESSAGE, json_string_new(message));
	if (ret == -1)
		goto error;

	ret = json_object_add(err, RESPONSE_ERROR_CODE, json_int_new(code));
	if (ret == -1)
		goto error;

	return err;

error:
	json_ref_put(err);
	return NULL;
}

static struct json_object *prepare_error_response(char *error_message, int error_code, struct json_object *id)
{
	struct json_object *err = create_error_object(error_message, error_code);
	if (err == NULL)
		return NULL;

	struct json_object *j_err = json_object_new();
	if (j_err == NULL)
		goto error;

	struct json_object *j_ver = json_string_new(JSON_RPC_VERSION);
	if (j_ver == NULL)
		goto error;

	int ret = json_object_add(j_err, REQUEST_VERSION, j_ver);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_err, RESPONSE_ERROR, err);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_err, REQUEST_ID, id);
	if (ret == -1) {
		json_ref_put(j_err);
		return NULL;
	}

	return j_err;

error:
	json_ref_put(err);
	json_ref_put(j_err);
	return NULL;
}

static struct json_rpc_list_entry *lookup_entry(struct json_rpc *jr, char *name)
{
	if (name == NULL)
		return NULL;

	struct json_rpc_list_entry *entry;
	list_for_each_entry(entry, &jr->methods, list) {
		if (!strcmp(entry->method_name, name))
			return entry;
	}

	return NULL;
}

static enum call_type eval_call_type(struct json_object *obj)
{
	if (json_type(obj) != json_type_object)
		return invalid_call;

	struct json_object *j_var;
	enum json_type type;

	j_var = json_object_get(obj, REQUEST_PARAMS);
	type = json_type(j_var);
	if (j_var != NULL && type != json_type_object && type != json_type_array)
		return invalid_call;

	j_var = json_object_get(obj, REQUEST_VERSION);
	type = json_type(j_var);
	if (j_var == NULL || type != json_type_string || strcmp(JSON_RPC_VERSION, json_string_get(j_var)))
		return invalid_call;

	j_var = json_object_get(obj, REQUEST_METHOD);
	type = json_type(j_var);
	if (j_var == NULL || type != json_type_string)
		return invalid_call;

	j_var = json_object_get(obj, REQUEST_ID);
	type = json_type(j_var);
	if (j_var != NULL && type != json_type_string && type != json_type_int)
		return invalid_call;
	else if (j_var == NULL)
		return notification_call;

	return request_call;
}

static void process_error(struct json_rpc *jr, char *err_message, int err_code, struct json_object *id)
{
	struct json_object *j_err = prepare_error_response(err_message, err_code, id);
	add_to_queue(jr->process_method, jr, j_err, jr->process_method_arg);
}

static void process_error_final(struct json_rpc *jr, char *err_message, int err_code, struct json_object *id)
{
	struct json_object *j_err = prepare_error_response(err_message, err_code, id);
	add_to_queue(jr->user_result_method, jr, j_err, jr->user_result_method_arg);
}

static void dispatch_call(struct json_rpc *jr, struct json_object *obj, json_rpc_method *notify_cb, json_rpc_method *req_cb, void *req_cb_arg)
{
	enum call_type type = eval_call_type(obj);

	if (type != notification_call) {
		jr->process_method = req_cb;
		jr->process_method_arg = req_cb_arg;
	} else
		jr->process_method = notify_cb;

	if (type == invalid_call) {
		process_error(jr, ERROR_INVALID_REQUEST, ERROR_INVALID_REQUEST_CODE, json_null_new());
		return;
	}

	jr->current_method_id = json_ref_get(json_object_get(obj, REQUEST_ID));

	struct json_rpc_list_entry *entry = lookup_entry(jr, json_string_get(json_object_get(obj, REQUEST_METHOD)));
	if (entry == NULL) {
		process_error(jr, ERROR_METHOD_NOT_FOUND, ERROR_METHOD_NOT_FOUND_CODE, jr->current_method_id);
		return;
	}

	add_to_queue(entry->method, jr, json_ref_get(json_object_get(obj, REQUEST_PARAMS)), entry->method_arg);
}

static void process_batched_notification(struct json_rpc *jr, struct json_object *j_res, void *arg);
static void process_batched_request(struct json_rpc *jr, struct json_object *j_res, void *arg);

static void process_batched_call_continue(struct json_rpc *jr)
{
	jr->current_batch_ind++;
	if (jr->current_batch_ind < jr->batch_length) {
		struct json_object *req = json_array_get(jr->batch, jr->current_batch_ind);
		dispatch_call(jr, req, process_batched_notification, process_batched_request, NULL);
		return;
	}

	if (json_array_length(jr->result_batch) > 0)
		add_to_queue(jr->user_result_method, jr, jr->result_batch, jr->user_result_method_arg);
	else
		json_ref_put(jr->result_batch);

	json_ref_put(jr->batch);
}

static void process_batched_request(struct json_rpc *jr, struct json_object *j_res, void *arg)
{
	int ret = json_array_add(jr->result_batch, j_res);
	if (ret == -1) {
		json_ref_put(jr->result_batch);
		process_error_final(jr, ERROR_INTERNAL, ERROR_INTERNAL_CODE, json_null_new());
		return;
	}

	process_batched_call_continue(jr);
}

static void process_batched_notification(struct json_rpc *jr, struct json_object *j_res, void *arg)
{
	json_ref_put(j_res);
	process_batched_call_continue(jr);
}

static void process_batched_call(struct json_rpc *jr, struct json_object *obj)
{
	jr->batch_length = json_array_length(obj);
	jr->batch = obj;
	jr->current_batch_ind = -1;
	jr->result_batch = json_array_new();
	if (jr->result_batch == NULL) {
		process_error_final(jr, ERROR_INTERNAL, ERROR_INTERNAL_CODE, json_null_new());
		return;
	}

	process_batched_call_continue(jr);
}

static void notify_result(struct json_rpc *jr, struct json_object *res, void *arg)
{
	json_ref_put(res);
}

static void process_single_call(struct json_rpc *jr, struct json_object *obj)
{
	dispatch_call(jr, obj, notify_result, jr->user_result_method, jr->user_result_method_arg);

	json_ref_put(obj);
}

void json_rpc_process(struct json_rpc *jr, struct json_object *obj, json_rpc_method *user_cb, void *user_cb_arg)
{
	jr->user_result_method = user_cb;
	jr->user_result_method_arg = user_cb_arg;

	if (json_type(obj) == json_type_object)
		process_single_call(jr, obj);
	else if (json_type(obj) == json_type_array && json_array_length(obj) > 0)
		process_batched_call(jr, obj);
	else
		process_error_final(jr, ERROR_INVALID_REQUEST, ERROR_INVALID_REQUEST_CODE, json_null_new());
}

void json_rpc_return(struct json_rpc *jr, struct json_object *res)
{
	struct json_object *json_rpc_res = prepare_response_object(jr->current_method_id, res);
	if (json_rpc_res == NULL && jr->current_method_id != NULL)
		process_error(jr, ERROR_INTERNAL, ERROR_INTERNAL_CODE, jr->current_method_id);
	else
		add_to_queue(jr->process_method, jr, json_rpc_res, jr->process_method_arg);
}
