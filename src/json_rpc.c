/*
 * json_rpc.c
 *
 *  Created on: Mar 4, 2011
 *      Author: ant
 */

#include "json_rpc.h"

#include "list.h"
#include "string_functions.h"
#include "log.h"

#include <event.h>
#include <string.h>

struct json_rpc {
	struct list_head methods;
	struct list_head result_cbs;
};

struct json_rpc_request {
	struct json_object *result_batch;
	int result_count;
	int waiting_result_count;

	struct json_rpc *jr;

	json_rpc_result user_cb;
	void *cb_arg;

	json_rpc_method return_cb;

	struct json_object *method_id;
};

enum call_type {request_call, notification_call, invalid_call};

struct json_rpc_list_entry {
	struct list_head list;

	json_rpc_method method;
	char *method_name;
	void *method_arg;
};

struct json_rpc_result_entry {
	struct list_head list;

	struct json_object *id;
	json_rpc_result result_cb;
	void *result_arg;
};

struct deferred_call_data {
	json_rpc_method method;
	struct json_object *obj;
	struct json_rpc_request *req;
	struct json_object *method_id;
	void *method_arg;
};

#define JSON_RPC_ID_KEY "id"
#define JSON_RPC_VERSION_KEY "jsonrpc"

#define JSON_RPC_VERSION "2.0"

#define REQUEST_PARAMS "params"
#define REQUEST_METHOD "method"

#define RESPONSE_ERROR "error"
#define RESPONSE_ERROR_MESSAGE "message"
#define RESPONSE_ERROR_CODE "code"
#define RESPONSE_RESULT "result"

#define ERROR_INVALID_REQUEST "Invalid request"
#define ERROR_INVALID_REQUEST_CODE -32600
#define ERROR_METHOD_NOT_FOUND "Method not found"
#define ERROR_METHOD_NOT_FOUND_CODE -32601
#define ERROR_INTERNAL "Internal error"
#define ERROR_INTERNAL_CODE -32603

static struct json_rpc_request *create_request(struct json_rpc *jr,
		json_rpc_method res_cb, json_rpc_result user_cb, void *cb_arg);
static void destroy_request(struct json_rpc_request *req);

static struct json_object *prepare_response_object(struct json_object *id, struct json_object *res);
static struct json_object *create_error_object(char *message, int code);
static struct json_object *prepare_error_response(char *error_message, int error_code, struct json_object *id);
static struct json_rpc_list_entry *lookup_entry(struct json_rpc *jr, char *name);
static enum call_type eval_call_type(struct json_object *obj);
static int is_response_single(struct json_object *resp);
static struct json_object *get_id(struct json_object *obj);

struct json_rpc *json_rpc_new()
{
	struct json_rpc *jr = (struct json_rpc *)malloc(sizeof(struct json_rpc));
	if (jr == NULL)
		return NULL;

	INIT_LIST_HEAD(&jr->methods);
	INIT_LIST_HEAD(&jr->result_cbs);

	return jr;
}

void json_rpc_free(struct json_rpc *jr)
{
	if (jr == NULL)
		return;

	struct list_head *p, *n;
	struct json_rpc_list_entry *list_entry;
	list_for_each_safe(p, n, &jr->methods) {
		list_entry = list_entry(p, struct json_rpc_list_entry, list);
		free(list_entry->method_name);
		list_del(p);
		free(list_entry);
	}

	struct json_rpc_result_entry *res_entry;
	list_for_each_safe(p, n, &jr->result_cbs) {
		res_entry = list_entry(p, struct json_rpc_result_entry, list);
		json_ref_put(res_entry->id);
		list_del(p);
		free(res_entry);
	}

	free(jr);
}

int json_rpc_add_method(struct json_rpc *jr, char *name, json_rpc_method method, void *arg)
{
	struct json_rpc_list_entry *entry = (struct json_rpc_list_entry *)malloc(sizeof(struct json_rpc_list_entry));
	if (entry == NULL) {
		log_info("%s : malloc_failed", __func__);
		return -1;
	}
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

static void deferred_callcb(int s, short t, void *arg)
{
	struct deferred_call_data *data = (struct deferred_call_data *)arg;
	struct json_rpc_request *req = data->req;

	if (req != NULL)
		req->method_id = data->method_id;

	data->method(req, data->obj, data->method_arg);

	free(data);
}

static void add_method_to_queue(json_rpc_method method, struct json_rpc_request *req,
		struct json_object *obj, void *arg)
{
	struct deferred_call_data *call_data = (struct deferred_call_data *)malloc(sizeof(struct deferred_call_data));
	if (call_data == NULL) {
		log_warn("%s : malloc failed. Method won`t dispath", __func__);
		return;
	}

	call_data->req = req;
	call_data->obj = obj;
	call_data->method = method;
	if (req != NULL)
		call_data->method_id = req->method_id;
	call_data->method_arg = arg;
	struct timeval tv = {0, 1};
	if (event_once(-1, EV_TIMEOUT, deferred_callcb, (void *)call_data, &tv) == -1)
		log_warn("%s : event_once failed. Method won`t dispath", __func__);
}

static void process_error(struct json_rpc_request *req, char *err_message, int err_code, struct json_object *id)
{
	struct json_object *j_err = prepare_error_response(err_message, err_code, id);
	if (j_err == NULL)
		log_warn("%s : prepare_error_response failed", __func__);

	add_method_to_queue(req->return_cb, req, j_err, req->cb_arg);
}

static void process_error_final(struct json_rpc *jr, json_rpc_result user_cb, void *arg, char *e_mess, int e_code)
{
	struct json_object *j_err = prepare_error_response(e_mess, e_code, json_null_new());
	if (j_err == NULL)
		log_warn("%s : prepare_error_response failed", __func__);

	user_cb(jr, j_err, arg);
}

static void dispatch_notification_call(struct json_rpc_request *req, struct json_object *obj)
{
	struct json_rpc_list_entry *entry = lookup_entry(req->jr, json_string_get(json_object_get(obj, REQUEST_METHOD)));

	if (entry != NULL)
		add_method_to_queue(entry->method, NULL, json_ref_get(json_object_get(obj, REQUEST_PARAMS)), entry->method_arg);
}

static void dispatch_request_call(struct json_rpc_request *req, struct json_object *obj)
{
	req->method_id = json_ref_get(json_object_get(obj, JSON_RPC_ID_KEY));

	struct json_rpc_list_entry *entry = lookup_entry(req->jr, json_string_get(json_object_get(obj, REQUEST_METHOD)));
	if (entry == NULL)
		process_error(req, ERROR_METHOD_NOT_FOUND, ERROR_METHOD_NOT_FOUND_CODE, req->method_id);
	else
		add_method_to_queue(entry->method, req, json_ref_get(json_object_get(obj, REQUEST_PARAMS)), entry->method_arg);
}

static enum call_type dispatch_call(struct json_rpc_request *req, struct json_object *obj)
{
	enum call_type type = eval_call_type(obj);

	if (type == invalid_call) {
		process_error(req, ERROR_INVALID_REQUEST, ERROR_INVALID_REQUEST_CODE, json_null_new());
		return invalid_call;
	}

	if (type == notification_call) {
		dispatch_notification_call(req, obj);
		return notification_call;
	}

	dispatch_request_call(req, obj);

	return request_call;
}

static void single_result_cb(struct json_rpc_request *req, struct json_object *j_res, void *arg)
{
	req->user_cb(req->jr, j_res, req->cb_arg);
	destroy_request(req);
}

static void batched_result_cb(struct json_rpc_request *req, struct json_object *j_res, void *arg)
{
	req->result_count++;

	int ret = json_array_add(req->result_batch, j_res);
	if (ret == -1)
		log_warn("%s : json_array_add failed", __func__);

	if (req->result_count != req->waiting_result_count)
		return;

	req->user_cb(req->jr, req->result_batch, req->cb_arg);
	destroy_request(req);
}

static int process_batched_call(struct json_rpc *jr, struct json_object *batch, json_rpc_result user_cb, void *cb_arg)
{
	struct json_rpc_request *req = create_request(jr, batched_result_cb, user_cb, cb_arg);
	if (req == NULL)
		return -1;

	int batch_length = json_array_length(batch);

	req->result_batch = json_array_new();
	if (req->result_batch == NULL) {
		destroy_request(req);
		return -1;
	}

	int i;
	for (i = 0; i < batch_length; i++) {
		if (dispatch_call(req, json_array_get(batch, i)) != notification_call)
			req->waiting_result_count++;
	}

	if (req->waiting_result_count == 0) {
		json_ref_put(req->result_batch);
		destroy_request(req);
	}

	return 0;
}

static int process_single_call(struct json_rpc *jr, struct json_object *obj, json_rpc_result user_cb, void *cb_arg)
{
	struct json_rpc_request *req = create_request(jr, single_result_cb, user_cb, cb_arg);
	if (req == NULL)
		return -1;

	if (dispatch_call(req, obj) == notification_call)
		destroy_request(req);

	return 0;
}

void json_rpc_process_request(struct json_rpc *jr, struct json_object *obj, json_rpc_result user_cb, void *cb_arg)
{
	int ret = 0;
	if (json_type(obj) == json_type_object)
		ret = process_single_call(jr, obj, user_cb, cb_arg);
	else if (json_type(obj) == json_type_array && json_array_length(obj) > 0)
		ret = process_batched_call(jr, obj, user_cb, cb_arg);
	else
		process_error_final(jr, user_cb, cb_arg, ERROR_INVALID_REQUEST, ERROR_INVALID_REQUEST_CODE);

	json_ref_put(obj);

	if (ret == -1)
		process_error_final(jr, user_cb, cb_arg, ERROR_INTERNAL, ERROR_INTERNAL_CODE);
}

void json_rpc_return(struct json_rpc_request *req, struct json_object *res)
{
	if (req == NULL)
		return;

	struct json_object *json_rpc_res = prepare_response_object(req->method_id, res);
	if (json_rpc_res == NULL) {
		log_info("%s : prepare_response_object failed", __func__);
		json_rpc_res = prepare_error_response(ERROR_INTERNAL, ERROR_INTERNAL_CODE, req->method_id);

		if (json_rpc_res == NULL)
			log_warn("%s : prepare_error_response failed", __func__);
	}

	add_method_to_queue(req->return_cb, req, json_rpc_res, req->cb_arg);
}

int json_rpc_preprocess_request(struct json_rpc *jr, struct json_object *req, json_rpc_result result, void *arg)
{
	struct json_object *id = get_id(req);
	if (id == NULL) {
		log_info("%s : can`t get ids from request", __func__);
		return -1;
	}

	struct list_head *p, *n;
	struct json_rpc_result_entry *res_entry;
	list_for_each_safe(p, n, &jr->result_cbs) {
		res_entry = list_entry(p, struct json_rpc_result_entry, list);
		if (!json_equals(res_entry->id, id)) {
			res_entry->result_cb = result;
			res_entry->result_arg = arg;
			json_ref_put(id);
			return 0;
		}
	}

	struct json_rpc_result_entry *entry = (struct json_rpc_result_entry *)
			malloc(sizeof(struct json_rpc_result_entry));
	if (entry == NULL) {
		log_info("%s : malloc_failed", __func__);
		json_ref_put(id);
		return -1;
	}

	entry->result_cb = result;
	entry->result_arg = arg;
	entry->id = id;

	list_add(&entry->list, &jr->result_cbs);

	return 0;
}

void json_rpc_process_response(struct json_rpc *jr, struct json_object *resp)
{
	if (!json_rpc_is_response(resp)) {
		log_info("%s : passed JSON isn`t JSON-RPC response", __func__);
		return;
	}

	struct json_object *id = get_id(resp);
	if (id == NULL)
		return;

	struct json_rpc_result_entry *pos, *res = NULL;
	list_for_each_entry(pos, &jr->result_cbs, list) {
		if (json_equals(pos->id, id) == 0) {
			res = pos;
			break;
		}
	}

	if (res == NULL) {
		json_ref_put(id);
		json_ref_put(resp);
		return;
	}

	list_del(&pos->list);

	pos->result_cb(jr, resp, pos->result_arg);

	json_ref_put(id);

	json_ref_put(pos->id);
	free(pos);
}

int json_rpc_is_response(struct json_object *resp)
{
	enum json_type type = json_type(resp);
	if (type == json_type_object)
		return is_response_single(resp);
	else if (type != json_type_array)
		return 0;

	int i, len;
	for (i = 0, len = json_array_length(resp); i < len; ++i)
		if (!is_response_single(json_array_get(resp, i)))
			return 0;

	return 1;
}

static struct json_rpc_request *create_request(struct json_rpc *jr,
		json_rpc_method res_cb, json_rpc_result user_cb, void *cb_arg)
{
	struct json_rpc_request *req = (struct json_rpc_request *)calloc(1, sizeof(struct json_rpc_request));
	if (req == NULL) {
		log_info("%s : malloc failed", __func__);
		return NULL;
	}

	req->return_cb = res_cb;
	req->user_cb = user_cb;
	req->cb_arg = cb_arg;
	req->jr = jr;

	return req;
}

static void destroy_request(struct json_rpc_request *req)
{
	free(req);
}

static struct json_object *prepare_response_object(struct json_object *id, struct json_object *res)
{
	struct json_object *j_res = json_object_new();
	if (j_res == NULL)
		goto error;

	struct json_object *j_ver = json_string_new(JSON_RPC_VERSION);
	if (j_ver == NULL)
		goto error;

	int ret = json_object_add(j_res, JSON_RPC_VERSION_KEY, j_ver);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_res, RESPONSE_RESULT, res);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_res, JSON_RPC_ID_KEY, id);
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

	int ret = json_object_add(j_err, JSON_RPC_VERSION_KEY, j_ver);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_err, RESPONSE_ERROR, err);
	if (ret == -1)
		goto error;

	ret = json_object_add(j_err, JSON_RPC_ID_KEY, id);
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

static struct json_object *get_id(struct json_object *req)
{
	enum json_type type = json_type(req);
	if (type == json_type_object)
		return json_ref_get(json_object_get(req, JSON_RPC_ID_KEY));
	else if (type != json_type_array)
		return NULL;

	struct json_object *ids = json_array_new();
	if (ids == NULL) {
		log_info("%s : can`t create array for id", __func__);
		return NULL;
	}

	int i, len;
	struct json_object *id;
	for (i = 0, len = json_array_length(req); i < len; ++i) {
		id = json_object_get(json_array_get(req, i), JSON_RPC_ID_KEY);
		if (id != NULL)
			json_array_add(ids, json_ref_get(id));
	}

	if (json_array_length(ids) == 0) {
		json_ref_put(ids);
		return NULL;
	}

	return ids;
}

static int is_response_single(struct json_object *resp)
{
	if (json_type(resp) != json_type_object)
		return 0;

	struct json_object *j_var, *j_err;
	enum json_type type;

	j_var = json_object_get(resp, JSON_RPC_VERSION_KEY);
	type = json_type(j_var);
	if (j_var == NULL || type != json_type_string || strcmp(JSON_RPC_VERSION, json_string_get(j_var)))
		return 0;

	j_var = json_object_get(resp, JSON_RPC_ID_KEY);
	type = json_type(j_var);
	if (j_var != NULL && 	type != json_type_string && type != json_type_int &&
							type != json_type_boolean && type != json_type_null)
		return 0;

	j_var = json_object_get(resp, RESPONSE_RESULT);
	type = json_type(j_var);
	if (j_var != NULL)
		return 1;

	j_err = json_object_get(resp, RESPONSE_ERROR);
	type = json_type(j_err);
	if (j_err == NULL || type != json_type_object)
		return 0;

	j_var = json_object_get(j_err, RESPONSE_ERROR_MESSAGE);
	type = json_type(j_var);
	if (j_var == NULL || (j_var != NULL && type != json_type_string))
		return 0;

	j_var = json_object_get(j_err, RESPONSE_ERROR_CODE);
	type = json_type(j_var);
	if (j_var == NULL || (j_var != NULL && type != json_type_int))
		return 0;

	return 1;
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

	j_var = json_object_get(obj, JSON_RPC_VERSION_KEY);
	type = json_type(j_var);
	if (j_var == NULL || type != json_type_string || strcmp(JSON_RPC_VERSION, json_string_get(j_var)))
		return invalid_call;

	j_var = json_object_get(obj, REQUEST_METHOD);
	type = json_type(j_var);
	if (j_var == NULL || type != json_type_string)
		return invalid_call;

	j_var = json_object_get(obj, JSON_RPC_ID_KEY);
	type = json_type(j_var);
	if (j_var != NULL && type != json_type_string && type != json_type_int)
		return invalid_call;
	else if (j_var == NULL)
		return notification_call;

	return request_call;
}
