/*
 * json.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "json/json.h"
#include "util/list.h"

#include <stdlib.h>

struct json_object {
	enum json_type type;
	char *key;
	int c_int;
	double c_double;
	char *c_string;
	boolean c_bool;
};



struct json_object *json_new_int(int i)
{
	return NULL;
}

struct json_object *json_new_double(double d)
{
	return NULL;
}

struct json_object *json_new_bool(boolean b)
{
	return NULL;
}

struct json_object *json_new_string(char *s)
{
	return NULL;
}

struct json_object *json_new_null()
{
	return NULL;
}


struct json_object *json_array_new()
{
	return NULL;
}

int json_array_length(struct json_object *obj)
{
	return 0;
}

struct json_object *json_array_get(struct json_object *obj, int i)
{
	return NULL;
}

int json_array_add(struct json_object *obj, struct json_object *val)
{
	return 0;
}

struct json_object *json_new_object()
{
	return NULL;
}

struct json_object *json_object_get(struct json_object *obj, char *key)
{
	return NULL;
}

struct json_object *json_object_del(struct json_object *obj, char *key)
{
	return NULL;
}

int json_object_add(struct json_object *obj, char *key, struct json_object *val)
{
	return NULL;
}

char *json_to_string(struct json_object *obj)
{
	return NULL;
}

enum json_type json_get_type(struct json_object *)
{
	return json_type_null;
}

void json_free(struct json_object *obj)
{
}
