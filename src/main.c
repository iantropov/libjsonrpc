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
//#include "json_parser/json_parser.h"

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

static int list_example(size_t jk)
{
	struct my_list entry, *new_entry;
	struct list_head *counter;
	INIT_LIST_HEAD(&entry.list);

	int i;

	for (i = 0; i < 1; i++) {
		new_entry = (struct my_list *)malloc(sizeof(struct my_list));
		if (new_entry == NULL)
			return EXIT_FAILURE;

		new_entry->value = i;
		list_add_tail(&(new_entry->list), &(entry.list));
	}

	struct list_head *next = entry.list.next, *prev = entry.list.prev;

	struct my_list *e1 = list_entry(next, struct my_list, list);
	struct my_list *e2 = list_entry(prev, struct my_list, list);


	list_for_each(counter, &entry.list) {
		new_entry = list_entry(counter, struct my_list, list);

		printf("Value : %d\n", new_entry->value);
	}

	return EXIT_SUCCESS;
}

static void check_as_invalid(char *s)
{
	struct json_object *j_int = json_parser_parse(s);
	fail_unless(j_int == NULL, "bad result");
}

int main()
{
	check_as_invalid("2-");
	check_as_invalid("2,");
	check_as_invalid("[2,");
	check_as_invalid(",");
	check_as_invalid("23 23");
	check_as_invalid("23,2");
	check_as_invalid("{2,2");
	check_as_invalid("{2,2}");
	check_as_invalid("{\"cd\":,}");
	check_as_invalid("");
	return 0;
}

