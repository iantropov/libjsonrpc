/*
 * parse.c
 *
 *  Created on: Aug 30, 2009
 *      Author: ant
 */

#include "json_parser.h"
#include "../util/string_functions.h"
#include "json_parser.h"
#include "../util/list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef int (add_function)(struct json_object *obj, char **key, struct json_object *value);

struct json_parser {
	struct json_object *cur_obj;
	struct list_head context_stack;
	add_function *add;

	int p_start;
	char *p_name;
};

#define STACK_SIZE 100


%%{
	machine json_parser;
	
	action A_final
	{
		is_end = 1;
	}
	action A_begin_object
	{
		tmp = json_object_new();
		ret = begin(&jp, tmp);
		if (ret == -1)
			fbreak;
		jp.cur_obj = tmp;
		jp.add = object_add;
		
		fcall object;
	}
	action A_begin_array
	{
		tmp = json_array_new();
		ret = begin(&jp, tmp);
		if (ret == -1)
			fbreak;
		jp.cur_obj = tmp;
		jp.add = array_add;
		
		fcall array;
	}
	action A_end_object
	{
		parser_pop_context(&jp);
		fret;
	}
	action A_end_array
	{
		parser_pop_context(&jp);
		fret;
	}
	action A_save_name
	{
		jp.p_name = get_interprete_str(&jp, p, buf);
		ret = jp.p_name == NULL ? -1 : 0;
		if (ret == -1)
			fbreak;
	}
	action A_save_float
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_double_new(atof(buf + jp.p_start)));
		if (ret == -1)
			fbreak;
	}
	action A_save_int
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_int_new(atoi(buf + jp.p_start)));
		if (ret == -1)
			fbreak;
	}
	action A_save_null
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_null_new());
		if (ret == -1)
			fbreak;
	}
	action A_save_true
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_boolean_new(TRUE));
		if (ret == -1)
			fbreak;
	}
	action A_save_false
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_boolean_new(FALSE));
		if (ret == -1)
			fbreak;
	}
	action A_save_string
	{
		tmp_str = get_interprete_str(&jp, p, buf);
		ret = jp.add(jp.cur_obj, &jp.p_name, json_string_new(tmp_str));
		free(tmp_str);		
		if (ret == -1)
			fbreak;
	}
	action A_err
	{
		fbreak;
	}
#-------------------------------------------------------------------------------------------
	action A_save_start
	{
		jp.p_start = (int)(p - buf);
	}
#------------------------------------------------------------------
	include json "json.rl";

	main := ( Request_json ) '\0' $err(A_err);

}%%

%% write data;

struct context_stack_entry {
	struct json_object *obj;
	add_function *add;

	struct list_head list;
};

static int array_add(struct json_object *obj, char **key, struct json_object *value)
{
	return json_array_add(obj, value);
}

static int object_add(struct json_object *obj, char **key, struct json_object *value)
{
	int ret = json_object_add(obj, *key, value);
	free(*key);
	*key = NULL;
	return ret;
}

static void parser_pop_context(struct json_parser *jp)
{
	if (list_empty(&jp->context_stack))
		return;

	struct list_head *next = jp->context_stack.next;
	struct context_stack_entry *entry = list_entry(next, struct context_stack_entry, list);
	
	jp->cur_obj = entry->obj;
	jp->add = entry->add;
	list_del(next);
	free(entry);
}

static int parser_push_context(struct json_parser *jp)
{
	struct context_stack_entry *new_entry = (struct context_stack_entry *)malloc(sizeof(struct context_stack_entry));
	if (new_entry == NULL)
		return -1;

	new_entry->obj = jp->cur_obj;
	new_entry->add = jp->add;
	list_add(&new_entry->list, &jp->context_stack);

	return 0;
}

static int begin(struct json_parser *jp, struct json_object *obj)
{
	int ret = jp->add(jp->cur_obj, &jp->p_name, obj);
	if (ret == -1)
		return -1;
	return parser_push_context(jp);
}

static int get_length(struct json_parser *jp, char *p, char *buf)
{
	int len = (int)(p - buf) - jp->p_start;
	return len;
}

static char *get_interprete_str(struct json_parser *jp, char *p, char *buf)
{
	int len = get_length(jp, p, buf);
	char *res = interpretate_esc_seq(buf + jp->p_start, len);
	return res;
}

static void json_parser_destroy(struct json_parser *jp)
{
	while(!list_empty(&jp->context_stack))
		parser_pop_context(jp);
	
	if (jp->p_name != NULL)
		free(jp->p_name);
	
	json_ref_put(jp->cur_obj);
}

static void json_parser_init(struct json_parser *jp)
{
	jp->cur_obj = json_array_new();
	jp->p_name = NULL;
	INIT_LIST_HEAD(&jp->context_stack);
	jp->add = array_add;
}

struct json_object *json_parser_parse(char *buf)
{
	int		cs = 1, 
			top = 0, 
			stack[STACK_SIZE], 
			ret = 0, 
			is_end = 0; 	
	char	*p = buf, 
			*pe = p + strlen(buf) + 1, 
			*eof = NULL,
			*tmp_str;		
	struct json_object *tmp;
	struct json_parser jp;
	
	json_parser_init(&jp);
			 	
	%% write exec;

	if (ret == -1 || is_end == 0) {
		json_parser_destroy(&jp);
		return NULL;
	}
	
	struct json_object *res_obj = json_array_get(jp.cur_obj, 0);
	json_ref_get(res_obj);
	
	json_parser_destroy(&jp);
	
	return res_obj;
}
