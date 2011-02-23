/*
 * json.h
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#ifndef JSON_H_
#define JSON_H_

struct json_object;

typedef char boolean;

enum json_type {json_type_null, json_type_int, json_type_double, json_type_boolean, json_type_string,
				json_type_object, json_type_array};

struct json_object *json_new_int(int i);
struct json_object *json_new_double(double d);
struct json_object *json_new_bool(boolean b);
struct json_object *json_new_string(char *s);
struct json_object *json_new_null();

struct json_object *json_array_new();
int json_array_length(struct json_object *obj);
struct json_object *json_array_get(struct json_object *obj, int i);
int json_array_add(struct json_object *obj, struct json_object *val);

struct json_object *json_new_object();
struct json_object *json_object_get(struct json_object *obj, char *key);
struct json_object *json_object_del(struct json_object *obj, char *key);
int json_object_add(struct json_object *obj, char *key, struct json_object *val);

char *json_to_string(struct json_object *obj);
enum json_type json_get_type(struct json_object *obj);
void json_free(struct json_object *obj);

#endif /* JSON_H_ */
