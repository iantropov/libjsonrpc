/*
 * main.c
 *
 *  Created on: Feb 23, 2011
 *      Author: ant
 */

#include "util/list.h"

#include <stdlib.h>
#include <stdio.h>

struct my_list {
	int value;
	unsigned long b;
	struct list_head list;
};

int main()
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

	unsigned long pos = (unsigned long)(&((struct my_list *)0)->list);

	printf("%d\n", pos);

	return EXIT_SUCCESS;
}

