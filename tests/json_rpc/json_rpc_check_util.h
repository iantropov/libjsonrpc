#ifndef JSON_RPC_CHECK_UTIL_H_
#define JSON_RPC_CHECK_UTIL_H_

#include "../../src/json/json.h"

struct json_object *create_single_request(struct json_object *method, struct json_object *params, struct json_object *id);
struct json_object *create_single_success_response(struct json_object *result, struct json_object *id);
struct json_object *create_error(char *message, int code);
struct json_object *create_single_error_response(struct json_object *j_err, struct json_object *id);
struct json_object *create_batched_request(struct json_object *methods, struct json_object *params, struct json_object *ids);
struct json_object *create_batched_response(struct json_object *results,struct json_object *ids);

#endif