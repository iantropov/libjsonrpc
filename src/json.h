/*
 * json.h
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#ifndef JSON_H_
#define JSON_H_

struct json_object;

#include <stdlib.h>

typedef char boolean;
#define FALSE (char)0
#define TRUE (char)1

enum json_type {json_type_null, json_type_int, json_type_double, json_type_boolean, json_type_string,
				json_type_object, json_type_array};

struct json_object *json_int_new(int i);
struct json_object *json_double_new(double d);
struct json_object *json_boolean_new(boolean b);
struct json_object *json_string_new(char *s);
struct json_object *json_string_new_len(char *s, size_t len);
struct json_object *json_null_new();

int json_int_get(struct json_object *obj);
double json_double_get(struct json_object *obj);
boolean json_boolean_get(struct json_object *obj);
char *json_string_get(struct json_object *obj);

struct json_object *json_array_new();
int json_array_length(struct json_object *obj);
struct json_object *json_array_get(struct json_object *obj, int i);
int json_array_add(struct json_object *obj, struct json_object *val);

struct json_object *json_object_new();
struct json_object *json_object_get(struct json_object *obj, char *key);
void json_object_del(struct json_object *obj, char *key);
int json_object_add(struct json_object *obj, char *key, struct json_object *val);

char *json_to_string(struct json_object *obj);
enum json_type json_type(struct json_object *obj);

void json_ref_put(struct json_object *obj);
struct json_object *json_ref_get(struct json_object *obj);

int json_equals(struct json_object *obj_1, struct json_object *obj_2);

#endif /* JSON_H_ */
