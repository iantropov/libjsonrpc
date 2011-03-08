/*
 * json.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "json.h"
#include "../util/list.h"
#include "../util/string_functions.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

struct json_data {
	int c_int;
	double c_double;
	char *c_string;
	boolean c_boolean;
	struct list_head array_list;
	struct list_head object_list;
};

typedef void (del_func)(struct json_object *obj);
typedef int (eq_func)(struct json_object *obj1, struct json_object *obj2);


struct json_object {
	enum json_type type;
	int ref_count;
	struct json_data data;
	del_func *delete;
	eq_func *equals;
};

struct array_list_entry {
	struct list_head list;
	struct json_object *obj;
};

struct object_list_entry {
	struct list_head list;
	struct json_object *obj;
	char *key;
};

#define JSON_INT_DEFAULT_VALUE 0
#define JSON_DOUBLE_DEFAULT_VALUE 0
#define JSON_BOOLEAN_DEFAULT_VALUE (char)0
#define JSON_STRING_DEFAULT_VALUE (char*)0
#define JSON_TYPE_DEFAULT_VALUE json_type_null
#define DEFAULT_ARRAY_LENGTH 0
#define DOUBLE_EPSILON 0.0000001

#define JSON_OBJECT_CREATE(type, member, df, ef, value) struct json_object *res = json_object_create(type, df, ef);\
	if (res == NULL) return NULL; res->data.member = value; return res;

#define JSON_OBJECT_GET(object, objtype, member, default_value) if(object==NULL||object->type!=objtype)return default_value;\
	return object->data.member;

static int int_equals(struct json_object *obj_1, struct json_object *obj_2);
static int string_equals(struct json_object *obj_1, struct json_object *obj_2);
static int double_equals(struct json_object *obj_1, struct json_object *obj_2);
static int boolean_equals(struct json_object *obj_1, struct json_object *obj_2);
static int array_equals(struct json_object *obj_1, struct json_object *obj_2);
static int object_equals(struct json_object *obj_1, struct json_object *obj_2);
static int null_equals(struct json_object *obj_1, struct json_object *obj_2);


static struct json_object *json_object_create(enum json_type type, del_func *delete, eq_func *equals)
{
	struct json_object *res = (struct json_object *)malloc(sizeof(struct json_object));
	if (res == NULL)
		return NULL;

	res->delete = delete;
	res->equals = equals;
	res->ref_count = 1;
	res->type = type;
	return res;
}

static void delete_primitive(struct json_object *obj)
{
	free(obj);
}

static void delete_string(struct json_object *obj)
{
	if (obj == NULL)
		return;
	free(obj->data.c_string);
	free(obj);
}

static void delete_array(struct json_object *obj)
{
	if (obj == NULL || obj->type != json_type_array)
		return;

	struct list_head *p, *n;
	struct array_list_entry *e;
	list_for_each_safe(p, n, &obj->data.array_list) {
		e = list_entry(p, struct array_list_entry, list);
		json_ref_put(e->obj);
		list_del(p);
		free(e);
	}

	free(obj);
}

static void delete_object(struct json_object *obj)
{
	if (obj == NULL || obj->type != json_type_object)
		return;

	struct list_head *p, *n;
	struct object_list_entry *e;
	list_for_each_safe(p, n, &obj->data.object_list) {
		e = list_entry(p, struct object_list_entry, list);
		json_ref_put(e->obj);
		list_del(p);
		free(e->key);
		free(e);
	}

	free(obj);
}

struct json_object *json_int_new(int i)
{
	JSON_OBJECT_CREATE(json_type_int, c_int, delete_primitive, int_equals, i);
}

struct json_object *json_double_new(double d)
{
	JSON_OBJECT_CREATE(json_type_double, c_double, delete_primitive, double_equals, d);
}

struct json_object *json_boolean_new(boolean b)
{
	JSON_OBJECT_CREATE(json_type_boolean, c_boolean, delete_primitive, boolean_equals, b);
}

struct json_object *json_string_new(char *s)
{
	char *str_copy = string_copy(s);
	if (str_copy == NULL)
		return NULL;

	JSON_OBJECT_CREATE(json_type_string, c_string, delete_string, string_equals, str_copy);
}

struct json_object *json_string_new_len(char *s, size_t len)
{
	char *str_copy = (char *)malloc(len + 1);
	if (str_copy == NULL)
		return NULL;
	strncpy(str_copy, s, len);
	str_copy[len] = '\0';

	JSON_OBJECT_CREATE(json_type_string, c_string, delete_string, string_equals, str_copy);
}

struct json_object *json_null_new()
{
	return json_object_create(json_type_null, delete_primitive, null_equals);
}

int json_int_get(struct json_object *obj)
{
	JSON_OBJECT_GET(obj, json_type_int, c_int, JSON_INT_DEFAULT_VALUE);
}

double json_double_get(struct json_object *obj)
{
	JSON_OBJECT_GET(obj, json_type_double, c_double, JSON_DOUBLE_DEFAULT_VALUE);
}

boolean json_boolean_get(struct json_object *obj)
{
	JSON_OBJECT_GET(obj, json_type_boolean, c_boolean, JSON_BOOLEAN_DEFAULT_VALUE);
}

char *json_string_get(struct json_object *obj)
{
	JSON_OBJECT_GET(obj, json_type_string, c_string, JSON_STRING_DEFAULT_VALUE);
}

struct json_object *json_array_new()
{
	struct json_object *res = json_object_create(json_type_array, delete_array, array_equals);
	if (res == NULL)
		return NULL;

	INIT_LIST_HEAD(&res->data.array_list);

	return res;
}

int json_array_length(struct json_object *obj)
{
	if (obj == NULL || obj->type != json_type_array)
		return DEFAULT_ARRAY_LENGTH;

	int length = 0;
	struct list_head *p;
	list_for_each(p, &obj->data.array_list) {
		length++;
	}

	return length;
}

struct json_object *json_array_get(struct json_object *obj, int i)
{
	if (obj == NULL || i < 0 || obj->type != json_type_array)
		return NULL;

	int count = 0;
	struct list_head *p;
	struct json_object *res = NULL;
	list_for_each(p, &obj->data.array_list) {
		if (count++ == i) {
			res = list_entry(p, struct array_list_entry, list)->obj;
			break;
		}
	}

	return res;
}

int json_array_add(struct json_object *obj, struct json_object *val)
{
	if (obj == NULL || val == NULL || obj->type != json_type_array)
		return -1;

	struct array_list_entry *new_entry = (struct array_list_entry *)malloc(sizeof(struct array_list_entry));
	if (new_entry == NULL)
		return -1;

	new_entry->obj = val;
	list_add_tail(&new_entry->list, &obj->data.array_list);

	return 0;
}

struct json_object *json_object_new()
{
	struct json_object *res = json_object_create(json_type_object, delete_object, object_equals);
	if (res == NULL)
		return NULL;

	INIT_LIST_HEAD(&res->data.object_list);

	return res;
}

struct json_object *json_object_get(struct json_object *obj, char *key)
{
	if (obj == NULL || key == NULL || obj->type != json_type_object)
		return NULL;

	struct object_list_entry *p;
	struct json_object *res = NULL;
	list_for_each_entry(p, &obj->data.object_list, list) {
		if (strcmp(p->key, key) == 0) {
			res = p->obj;
			break;
		}
	}

	return res;
}

void json_object_del(struct json_object *obj, char *key)
{
	if (obj == NULL || key == NULL || obj->type != json_type_object)
		return;

	struct list_head *p, *n;
	struct object_list_entry *entry;
	list_for_each_safe(p, n, &obj->data.object_list) {
		entry = list_entry(p, struct object_list_entry, list);
		if (strcmp(entry->key, key) == 0) {
			list_del(p);
			free(entry->key);
			json_ref_put(entry->obj);
			free(entry);
			break;
		}
	}
}

int json_object_add(struct json_object *obj, char *key, struct json_object *val)
{
	if (obj == NULL || val == NULL || key == NULL || obj->type != json_type_object)
		return -1;

	json_object_del(obj, key);//remove value with same key

	struct object_list_entry *new_entry = (struct object_list_entry *)malloc(sizeof(struct object_list_entry));
	if (new_entry == NULL)
		return -1;

	char *new_key = string_copy(key);
	if (new_key == NULL)
		return -1;

	new_entry->obj = val;
	new_entry->key = new_key;
	list_add(&new_entry->list, &obj->data.object_list);

	return 0;
}

char *json_to_string(struct json_object *obj)
{
	return NULL;
}

enum json_type json_type(struct json_object *obj)
{
	return (obj == NULL) ? JSON_TYPE_DEFAULT_VALUE : obj->type;
}

void json_ref_put(struct json_object *obj)
{
	if (obj == NULL)
		return;

	obj->ref_count--;
	if (obj->ref_count == 0)
		obj->delete(obj);
}

struct json_object *json_ref_get(struct json_object *obj)
{
	if (obj == NULL)
		return NULL;

	obj->ref_count++;
	return obj;
}

static int int_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_int || obj_2->data.c_int != obj_1->data.c_int)
		return -1;

	return 0;
}

static int boolean_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_boolean || obj_2->data.c_boolean != obj_1->data.c_boolean)
		return -1;

	return 0;
}

static int string_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_string || strcmp(obj_2->data.c_string, obj_1->data.c_string))
		return -1;

	return 0;
}

static int double_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_double || fabs(obj_1->data.c_double - obj_2->data.c_double) >= DOUBLE_EPSILON)
		return -1;

	return 0;
}

static int null_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_null)
		return -1;

	return 0;
}

static int array_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_array)
		return -1;

	int len_1 = json_array_length(obj_1);
	int len_2 = json_array_length(obj_2);
	if (len_1 != len_2)
		return -1;

	struct array_list_entry *p;
	int count = 0;
	list_for_each_entry(p, &obj_1->data.array_list, list)
		if (p->obj->equals(p->obj, json_array_get(obj_2, count++)) != 0)
			return -1;

	return 0;
}

static int object_length(struct json_object *obj)
{
	int len = 0;
	struct list_head *p;
	list_for_each(p, &obj->data.object_list)
		len++;

	return len;
}

static int object_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_2->type != json_type_object)
		return -1;

	int len_1 = object_length(obj_1);
	int len_2 = object_length(obj_2);
	if (len_1 != len_2)
		return -1;

	struct object_list_entry *p;
	struct json_object *tmp;
	list_for_each_entry(p, &obj_1->data.object_list, list) {
		tmp = json_object_get(obj_2, p->key);
		if (tmp == NULL || p->obj->equals(p->obj, json_object_get(obj_2, p->key)) != 0)
			return -1;
	}

	return 0;
}

int json_equals(struct json_object *obj_1, struct json_object *obj_2)
{
	if (obj_1 == NULL || obj_2 == NULL)
		return -1;

	return obj_1->equals(obj_1, obj_2);
}
