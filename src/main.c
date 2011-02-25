/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "util/list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "json/json.h"

struct my_list {
	int value;
	unsigned long b;
	struct list_head list;
};

#define CHECK_INT 5
#define CHECK_STR "cdcd"

static void fail_unless(boolean b, char *s)
{
	if (!b)
		printf("%s\n", s);
}

static int list_example()
{
	struct my_list entry, *new_entry;
	struct list_head *counter;
	INIT_LIST_HEAD(&entry.list);

	int i;

	for (i = 0; i < 5; i++) {
		new_entry = (struct my_list *)malloc(sizeof(struct my_list));
		if (new_entry == NULL)
			return EXIT_FAILURE;

		new_entry->value = i;
		list_add_tail(&(new_entry->list), &(entry.list));
	}

	list_for_each(counter, &entry.list) {
		new_entry = list_entry(counter, struct my_list, list);

		printf("Value : %d\n", new_entry->value);
	}

	return EXIT_SUCCESS;
}

int main()
{
	struct json_object *obj_0 = json_object_new();
	struct json_object *ar_0 = json_array_new();
	struct json_object *obj_1 = json_object_new();

	struct json_object *j_int = json_int_new(CHECK_INT);
	struct json_object *j_str = json_string_new(CHECK_STR);
	struct json_object *j_bool = json_boolean_new(TRUE);

	json_object_add(obj_1, "ar_0", ar_0);
	json_object_add(obj_0, "obj_1", obj_1);
	json_object_add(obj_0, "j_bool", j_bool);
	json_object_add(obj_1, "j_int", j_int);
	json_array_add(ar_0, json_ref_get(j_str));

	fail_unless(json_array_length(ar_0) == 1, "bad length");

	fail_unless(strcmp(json_string_get(json_array_get(json_object_get(json_object_get(obj_0,
				"obj_1"), "ar_0"), 0)), CHECK_STR) == 0, "bad str in obj");

	fail_unless(json_int_get(json_object_get(json_object_get(obj_0,
					"obj_1"), "j_int")) == CHECK_INT, "bad int in obj");

	fail_unless(json_boolean_get(json_object_get(obj_0, "j_bool")) == TRUE, "bad bool in obj");

	json_ref_put(obj_0);

	json_ref_put(j_str);

	return EXIT_SUCCESS;
}

