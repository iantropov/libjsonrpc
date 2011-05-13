/*
 * parse.c
 *
 *  Created on: Aug 30, 2009
 *      Author: ant
 */

#include "json_parser.h"
#include "string_functions.h"
#include "json_parser.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <event.h>

typedef int (add_function)(struct json_object *obj, char **key, struct json_object *value);

#define STACK_SIZE 1024

struct json_parser {
	struct json_object *cur_obj;
	struct list_head context_stack;
	add_function *add;

	int p_start;
	char *p_name;
	
	int off;
	int cs;
	int top;
	int stack[STACK_SIZE];
	
	json_success_cb s_cb;
	json_error_cb e_cb;
	void *cb_arg;
	
	short error;
	
	int ready_for_drain;
};

static void handle_json_object(struct json_parser *jp);
static void handle_error(struct json_parser *jp, short what);
static void update_for_drain(struct json_parser *jp, char *cur, char *start);

%%{
	machine json_parser;
	
	action A_final
	{
		handle_json_object(jp);
	}
	action A_begin_object
	{
		tmp = json_object_new();
		ret = begin(jp, tmp);
		if (ret == -1)
			fbreak;
		jp->cur_obj = tmp;
		jp->add = object_add;
		
		fcall object;
	}
	action A_begin_array
	{
		tmp = json_array_new();
		ret = begin(jp, tmp);
		if (ret == -1)
			fbreak;
		jp->cur_obj = tmp;
		jp->add = array_add;
		
		fcall array;
	}
	action A_end_object
	{
		update_for_drain(jp, p, start);
		
		parser_pop_context(jp);
		fret;
	}
	action A_end_array
	{
		update_for_drain(jp, p, start);
		
		parser_pop_context(jp);
		fret;
	}
	action A_save_name
	{
		jp->p_name = get_interprete_str(jp, p, start);
		ret = jp->p_name == NULL ? -1 : 0;
		if (ret == -1)
			fbreak;
	}
	action A_save_float
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_double_new(atof(start + jp->p_start)));
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_save_int
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_int_new(atoi(start + jp->p_start)));
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_save_null
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_null_new());
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_save_true
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_boolean_new(TRUE));
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_save_false
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_boolean_new(FALSE));
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_save_string
	{
		tmp_str = get_interprete_str(jp, p, start);
		ret = jp->add(jp->cur_obj, &jp->p_name, json_string_new(tmp_str));
		free(tmp_str);
		update_for_drain(jp, p, start);
		if (ret == -1)
			fbreak;
	}
	action A_err
	{
		handle_error(jp, JSON_PARSE_ERROR);
		fbreak;
	}
#-------------------------------------------------------------------------------------------
	action A_save_start
	{
		jp->p_start = (int)(p - start);
	}
#------------------------------------------------------------------
	include json "json.rl";

	single_json_value := json_value %A_final '\0' $err(A_err);
	main := (non_plain_json_value %to(A_final))+ $err(A_err);

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

static int get_length(struct json_parser *jp, char *p, char *start)
{
	int len = (int)(p - start) - jp->p_start;
	return len;
}

static char *get_interprete_str(struct json_parser *jp, char *p, char *start)
{
	int len = get_length(jp, p, start);
	char *res = interpretate_esc_seq(start + jp->p_start, len);
	return res;
}

void json_parser_destroy(struct json_parser *jp)
{
	while(!list_empty(&jp->context_stack))
		parser_pop_context(jp);
	
	if (jp->p_name != NULL)
		free(jp->p_name);
	
	json_ref_put(jp->cur_obj);
}

static int json_parser_init(struct json_parser *jp, json_success_cb s_cb, json_error_cb e_cb, void *arg)
{
	bzero(jp, sizeof(struct json_parser));
	
	jp->cur_obj = json_array_new();
	if (jp->cur_obj == NULL)
		return -1;
	
	jp->p_name = NULL;
	INIT_LIST_HEAD(&jp->context_stack);
	jp->add = array_add;
	
	jp->s_cb = s_cb;
	jp->e_cb = e_cb;
	jp->cb_arg = arg;
	
	jp->cs = 1;
	
	return 0;
}

struct json_parser *json_parser_new(json_success_cb s_cb, json_error_cb e_cb, void *arg)
{
	struct json_parser *jp = (struct json_parser *)malloc(sizeof(struct json_parser));
	if (jp == NULL)
		return NULL;
	
	if (json_parser_init(jp, s_cb, e_cb, arg) == -1)
		return NULL;
	
	return jp;
}

static void json_object_cb(struct json_parser *jp, struct json_object *obj, void *arg)
{
	struct json_object **res_obj = arg; 
	
	*res_obj = obj;
}

struct json_object *json_parser_parse(char *buf)
{
	struct json_parser jp;
	struct json_object *obj = NULL;
	if (json_parser_init(&jp, json_object_cb, NULL, &obj) == -1)
		return NULL;
	
	jp.cs = json_parser_en_single_json_value;
	json_parser_process(&jp, buf, buf + strlen(buf)+1);
	
	json_parser_destroy(&jp);
	
	return obj;
}

void json_parser_process(struct json_parser *jp, char *start, char *end)
{
	int		cs = jp->cs, 
			top = jp->top, 
			*stack = jp->stack, 
			ret = 0;
	char	*p = start + jp->off, 
			*pe = end, 
			*tmp_str,
			*eof = NULL;
	struct json_object *tmp;
	
	%% write exec;
	
	if (ret == -1)
		handle_error(jp, JSON_INTERNAL_ERROR);
	
	jp->cs = cs;
	jp->top = top;
	jp->off = (int)(p - start);
}

static void handle_json_object(struct json_parser *jp)
{
	struct json_object *res_obj = json_array_get(jp->cur_obj, 0);
	json_ref_get(res_obj);
	json_array_del(jp->cur_obj, 0);
	
	if (jp->s_cb != NULL)
		jp->s_cb(jp, res_obj, jp->cb_arg);
}

static void handle_errorcb(int fd, short what, void *arg)
{
	struct json_parser *jp = arg;
	
	jp->e_cb(jp, jp->error, jp->cb_arg);
}
	
static void handle_error(struct json_parser *jp, short what)
{
	jp->error |= what;
	
	if (jp->e_cb == NULL)
		return;
	
	struct timeval tv = {0, 1};
	event_once(-1, EV_TIMEOUT, handle_errorcb, jp, &tv);
}

static void update_for_drain(struct json_parser *jp, char *cur, char *start)
{
	jp->ready_for_drain = (int)(cur - start) + 1;
}

void json_parser_free(struct json_parser *jp)
{
	json_parser_destroy(jp);
	
	free(jp);
}

int json_parser_drain(struct json_parser *jp)
{
	int ret = jp->ready_for_drain;
	
	jp->off -= jp->ready_for_drain;
	jp->ready_for_drain = 0;

	return ret;
}