
#line 1 "json_parser.rl"
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


#line 158 "json_parser.rl"



#line 56 "json_parser.c"
static const char _json_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 2, 6, 0, 
	2, 6, 3, 2, 6, 4, 2, 7, 
	0, 2, 7, 3, 2, 7, 4, 2, 
	13, 5, 2, 13, 11
};

static const short _json_parser_key_offsets[] = {
	0, 0, 2, 5, 15, 17, 27, 29, 
	31, 33, 43, 53, 55, 65, 67, 69, 
	72, 74, 83, 89, 95, 101, 107, 109, 
	117, 119, 126, 130, 132, 137, 138, 139, 
	140, 141, 144, 145, 146, 147, 150, 151, 
	152, 153, 156, 165, 171, 177, 183, 189, 
	200, 210, 212, 222, 224, 226, 229, 239, 
	241, 249, 251, 258, 262, 264, 269, 270, 
	271, 272, 273, 276, 277, 278, 279, 282, 
	283, 284, 285, 288, 297, 303, 309, 315, 
	321, 330, 340, 342, 352, 354, 356, 357, 
	366, 372, 378, 384, 390, 392, 398, 400, 
	405, 409, 411, 414, 415, 416, 417, 418, 
	419, 420, 421, 422, 423, 424, 425, 426, 
	427, 429, 429, 429
};

static const char _json_parser_trans_keys[] = {
	91, 123, 32, 34, 125, 34, 92, -64, 
	-33, -32, -17, -16, -9, 32, 126, -128, 
	-65, 34, 92, -64, -33, -32, -17, -16, 
	-9, 32, 126, -128, -65, -128, -65, 32, 
	58, 32, 34, 45, 91, 102, 110, 116, 
	123, 48, 57, 34, 92, -64, -33, -32, 
	-17, -16, -9, 32, 126, -128, -65, 34, 
	92, -64, -33, -32, -17, -16, -9, 32, 
	126, -128, -65, -128, -65, 32, 44, 125, 
	32, 34, 34, 47, 92, 98, 102, 110, 
	114, 116, 117, 48, 57, 65, 70, 97, 
	102, 48, 57, 65, 70, 97, 102, 48, 
	57, 65, 70, 97, 102, 48, 57, 65, 
	70, 97, 102, 48, 57, 32, 44, 46, 
	69, 101, 125, 48, 57, 48, 57, 32, 
	44, 69, 101, 125, 48, 57, 43, 45, 
	48, 57, 48, 57, 32, 44, 125, 48, 
	57, 97, 108, 115, 101, 32, 44, 125, 
	117, 108, 108, 32, 44, 125, 114, 117, 
	101, 32, 44, 125, 34, 47, 92, 98, 
	102, 110, 114, 116, 117, 48, 57, 65, 
	70, 97, 102, 48, 57, 65, 70, 97, 
	102, 48, 57, 65, 70, 97, 102, 48, 
	57, 65, 70, 97, 102, 32, 34, 45, 
	91, 93, 102, 110, 116, 123, 48, 57, 
	34, 92, -64, -33, -32, -17, -16, -9, 
	32, 126, -128, -65, 34, 92, -64, -33, 
	-32, -17, -16, -9, 32, 126, -128, -65, 
	-128, -65, 32, 44, 93, 32, 34, 45, 
	91, 102, 110, 116, 123, 48, 57, 48, 
	57, 32, 44, 46, 69, 93, 101, 48, 
	57, 48, 57, 32, 44, 69, 93, 101, 
	48, 57, 43, 45, 48, 57, 48, 57, 
	32, 44, 93, 48, 57, 97, 108, 115, 
	101, 32, 44, 93, 117, 108, 108, 32, 
	44, 93, 114, 117, 101, 32, 44, 93, 
	34, 47, 92, 98, 102, 110, 114, 116, 
	117, 48, 57, 65, 70, 97, 102, 48, 
	57, 65, 70, 97, 102, 48, 57, 65, 
	70, 97, 102, 48, 57, 65, 70, 97, 
	102, 34, 45, 91, 102, 110, 116, 123, 
	48, 57, 34, 92, -64, -33, -32, -17, 
	-16, -9, 32, 126, -128, -65, 34, 92, 
	-64, -33, -32, -17, -16, -9, 32, 126, 
	-128, -65, -128, -65, 0, 34, 47, 92, 
	98, 102, 110, 114, 116, 117, 48, 57, 
	65, 70, 97, 102, 48, 57, 65, 70, 
	97, 102, 48, 57, 65, 70, 97, 102, 
	48, 57, 65, 70, 97, 102, 48, 57, 
	0, 46, 69, 101, 48, 57, 48, 57, 
	0, 69, 101, 48, 57, 43, 45, 48, 
	57, 48, 57, 0, 48, 57, 97, 108, 
	115, 101, 0, 117, 108, 108, 0, 114, 
	117, 101, 0, 91, 123, 0
};

static const char _json_parser_single_lengths[] = {
	0, 2, 3, 2, 0, 2, 0, 0, 
	2, 8, 2, 0, 2, 0, 0, 3, 
	2, 9, 0, 0, 0, 0, 0, 6, 
	0, 5, 2, 0, 3, 1, 1, 1, 
	1, 3, 1, 1, 1, 3, 1, 1, 
	1, 3, 9, 0, 0, 0, 0, 9, 
	2, 0, 2, 0, 0, 3, 8, 0, 
	6, 0, 5, 2, 0, 3, 1, 1, 
	1, 1, 3, 1, 1, 1, 3, 1, 
	1, 1, 3, 9, 0, 0, 0, 0, 
	7, 2, 0, 2, 0, 0, 1, 9, 
	0, 0, 0, 0, 0, 4, 0, 3, 
	2, 0, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	2, 0, 0, 0
};

static const char _json_parser_range_lengths[] = {
	0, 0, 0, 4, 1, 4, 1, 1, 
	0, 1, 4, 1, 4, 1, 1, 0, 
	0, 0, 3, 3, 3, 3, 1, 1, 
	1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 3, 3, 3, 3, 1, 
	4, 1, 4, 1, 1, 0, 1, 1, 
	1, 1, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 3, 3, 3, 3, 
	1, 4, 1, 4, 1, 1, 0, 0, 
	3, 3, 3, 3, 1, 1, 1, 1, 
	1, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0
};

static const short _json_parser_index_offsets[] = {
	0, 0, 3, 7, 14, 16, 23, 25, 
	27, 30, 40, 47, 49, 56, 58, 60, 
	64, 67, 77, 81, 85, 89, 93, 95, 
	103, 105, 112, 116, 118, 123, 125, 127, 
	129, 131, 135, 137, 139, 141, 145, 147, 
	149, 151, 155, 165, 169, 173, 177, 181, 
	192, 199, 201, 208, 210, 212, 216, 226, 
	228, 236, 238, 245, 249, 251, 256, 258, 
	260, 262, 264, 268, 270, 272, 274, 278, 
	280, 282, 284, 288, 298, 302, 306, 310, 
	314, 323, 330, 332, 339, 341, 343, 345, 
	355, 359, 363, 367, 371, 373, 379, 381, 
	386, 390, 392, 395, 397, 399, 401, 403, 
	405, 407, 409, 411, 413, 415, 417, 419, 
	421, 424, 425, 426
};

static const unsigned char _json_parser_indicies[] = {
	1, 2, 0, 3, 4, 5, 0, 10, 
	11, 6, 7, 8, 9, 0, 12, 0, 
	16, 17, 13, 14, 15, 12, 0, 13, 
	0, 14, 0, 18, 19, 0, 19, 20, 
	21, 23, 24, 25, 26, 27, 22, 0, 
	32, 33, 28, 29, 30, 31, 0, 34, 
	0, 38, 39, 35, 36, 37, 34, 0, 
	35, 0, 36, 0, 40, 41, 5, 0, 
	41, 4, 0, 34, 34, 34, 34, 34, 
	34, 34, 34, 42, 0, 43, 43, 43, 
	0, 44, 44, 44, 0, 45, 45, 45, 
	0, 34, 34, 34, 0, 46, 0, 47, 
	48, 49, 50, 50, 51, 46, 0, 52, 
	0, 53, 54, 50, 50, 55, 52, 0, 
	56, 56, 57, 0, 57, 0, 53, 54, 
	55, 57, 0, 58, 0, 59, 0, 60, 
	0, 61, 0, 40, 41, 5, 0, 62, 
	0, 63, 0, 64, 0, 40, 41, 5, 
	0, 65, 0, 66, 0, 67, 0, 40, 
	41, 5, 0, 12, 12, 12, 12, 12, 
	12, 12, 12, 68, 0, 69, 69, 69, 
	0, 70, 70, 70, 0, 71, 71, 71, 
	0, 12, 12, 12, 0, 72, 73, 74, 
	76, 77, 78, 79, 80, 81, 75, 0, 
	86, 87, 82, 83, 84, 85, 0, 88, 
	0, 92, 93, 89, 90, 91, 88, 0, 
	89, 0, 90, 0, 94, 95, 77, 0, 
	95, 73, 74, 76, 78, 79, 80, 81, 
	75, 0, 96, 0, 97, 98, 99, 100, 
	101, 100, 96, 0, 102, 0, 103, 104, 
	100, 105, 100, 102, 0, 106, 106, 107, 
	0, 107, 0, 103, 104, 105, 107, 0, 
	108, 0, 109, 0, 110, 0, 111, 0, 
	94, 95, 77, 0, 112, 0, 113, 0, 
	114, 0, 94, 95, 77, 0, 115, 0, 
	116, 0, 117, 0, 94, 95, 77, 0, 
	88, 88, 88, 88, 88, 88, 88, 88, 
	118, 0, 119, 119, 119, 0, 120, 120, 
	120, 0, 121, 121, 121, 0, 88, 88, 
	88, 0, 122, 124, 126, 127, 128, 129, 
	130, 125, 123, 135, 136, 131, 132, 133, 
	134, 123, 137, 123, 141, 142, 138, 139, 
	140, 137, 123, 138, 123, 139, 123, 143, 
	0, 137, 137, 137, 137, 137, 137, 137, 
	137, 144, 123, 145, 145, 145, 123, 146, 
	146, 146, 123, 147, 147, 147, 123, 137, 
	137, 137, 123, 148, 123, 149, 150, 151, 
	151, 148, 0, 152, 123, 153, 151, 151, 
	152, 0, 154, 154, 155, 123, 155, 123, 
	153, 155, 0, 156, 123, 157, 123, 158, 
	123, 159, 123, 143, 0, 160, 123, 161, 
	123, 162, 123, 143, 0, 163, 123, 164, 
	123, 165, 123, 143, 0, 1, 2, 0, 
	0, 0, 0, 0
};

static const char _json_parser_trans_targs[] = {
	0, 112, 112, 2, 3, 113, 4, 6, 
	7, 5, 8, 42, 5, 4, 6, 7, 
	8, 42, 8, 9, 10, 22, 23, 15, 
	29, 34, 38, 15, 11, 13, 14, 12, 
	15, 17, 12, 11, 13, 14, 15, 17, 
	15, 16, 18, 19, 20, 21, 23, 15, 
	16, 24, 26, 113, 25, 15, 16, 113, 
	27, 28, 30, 31, 32, 33, 35, 36, 
	37, 39, 40, 41, 43, 44, 45, 46, 
	47, 48, 55, 56, 53, 114, 62, 67, 
	71, 53, 49, 51, 52, 50, 53, 75, 
	50, 49, 51, 52, 53, 75, 53, 54, 
	56, 53, 54, 57, 59, 114, 58, 53, 
	54, 114, 60, 61, 63, 64, 65, 66, 
	68, 69, 70, 72, 73, 74, 76, 77, 
	78, 79, 81, 0, 92, 93, 86, 99, 
	104, 108, 86, 82, 84, 85, 83, 86, 
	87, 83, 82, 84, 85, 86, 87, 115, 
	88, 89, 90, 91, 93, 115, 94, 96, 
	95, 115, 97, 98, 100, 101, 102, 103, 
	105, 106, 107, 109, 110, 111
};

static const char _json_parser_trans_actions[] = {
	25, 5, 3, 0, 0, 7, 27, 27, 
	27, 27, 47, 27, 0, 0, 0, 0, 
	11, 0, 0, 0, 0, 27, 27, 5, 
	0, 0, 0, 3, 27, 27, 27, 27, 
	50, 27, 0, 0, 0, 0, 23, 0, 
	0, 0, 0, 0, 0, 0, 0, 15, 
	15, 0, 0, 41, 0, 13, 13, 32, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 27, 27, 5, 9, 0, 0, 
	0, 3, 27, 27, 27, 27, 50, 27, 
	0, 0, 0, 0, 23, 0, 0, 0, 
	0, 15, 15, 0, 0, 44, 0, 13, 
	13, 35, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 27, 27, 5, 0, 
	0, 0, 3, 27, 27, 27, 27, 50, 
	27, 0, 0, 0, 0, 23, 0, 1, 
	0, 0, 0, 0, 0, 38, 0, 0, 
	0, 29, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const char _json_parser_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 21, 0, 0, 0, 17, 0, 0, 
	0, 19, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 21, 0, 0, 0, 17, 0, 
	0, 0, 19, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 21, 
	0, 0, 0, 17, 0, 0, 0, 19, 
	1, 0, 0, 0
};

static const char _json_parser_eof_actions[] = {
	0, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	0, 0, 0, 0, 0, 0, 25, 0, 
	0, 0, 0, 0, 0, 25, 0, 25, 
	0, 0, 25, 0, 0, 0, 0, 25, 
	0, 0, 0, 25, 0, 0, 0, 25, 
	0, 0, 0, 0
};

static const int json_parser_start = 1;
static const int json_parser_first_final = 112;
static const int json_parser_error = 0;

static const int json_parser_en_object = 2;
static const int json_parser_en_array = 47;
static const int json_parser_en_single_json_value = 80;
static const int json_parser_en_main = 1;


#line 161 "json_parser.rl"

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
	
	
#line 496 "json_parser.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _json_parser_trans_keys + _json_parser_key_offsets[cs];
	_trans = _json_parser_index_offsets[cs];

	_klen = _json_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _json_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _json_parser_indicies[_trans];
	cs = _json_parser_trans_targs[_trans];

	if ( _json_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _json_parser_actions + _json_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 52 "json_parser.rl"
	{
		handle_json_object(jp);
	}
	break;
	case 1:
#line 56 "json_parser.rl"
	{
		tmp = json_object_new();
		ret = begin(jp, tmp);
		if (ret == -1)
			{p++; goto _out; }
		jp->cur_obj = tmp;
		jp->add = object_add;
		
		{stack[top++] = cs; cs = 2; goto _again;}
	}
	break;
	case 2:
#line 67 "json_parser.rl"
	{
		tmp = json_array_new();
		ret = begin(jp, tmp);
		if (ret == -1)
			{p++; goto _out; }
		jp->cur_obj = tmp;
		jp->add = array_add;
		
		{stack[top++] = cs; cs = 47; goto _again;}
	}
	break;
	case 3:
#line 78 "json_parser.rl"
	{
		update_for_drain(jp, p, start);
		
		parser_pop_context(jp);
		{cs = stack[--top]; goto _again;}
	}
	break;
	case 4:
#line 85 "json_parser.rl"
	{
		update_for_drain(jp, p, start);
		
		parser_pop_context(jp);
		{cs = stack[--top]; goto _again;}
	}
	break;
	case 5:
#line 92 "json_parser.rl"
	{
		jp->p_name = get_interprete_str(jp, p, start);
		ret = jp->p_name == NULL ? -1 : 0;
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 6:
#line 99 "json_parser.rl"
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_double_new(atof(start + jp->p_start)));
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 7:
#line 106 "json_parser.rl"
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_int_new(atoi(start + jp->p_start)));
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 11:
#line 134 "json_parser.rl"
	{
		tmp_str = get_interprete_str(jp, p, start);
		ret = jp->add(jp->cur_obj, &jp->p_name, json_string_new(tmp_str));
		free(tmp_str);
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 12:
#line 143 "json_parser.rl"
	{
		handle_error(jp, JSON_PARSE_ERROR);
		{p++; goto _out; }
	}
	break;
	case 13:
#line 149 "json_parser.rl"
	{
		jp->p_start = (int)(p - start);
	}
	break;
#line 671 "json_parser.c"
		}
	}

_again:
	_acts = _json_parser_actions + _json_parser_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 0:
#line 52 "json_parser.rl"
	{
		handle_json_object(jp);
	}
	break;
	case 8:
#line 113 "json_parser.rl"
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_null_new());
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 9:
#line 120 "json_parser.rl"
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_boolean_new(TRUE));
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 10:
#line 127 "json_parser.rl"
	{
		ret = jp->add(jp->cur_obj, &jp->p_name, json_boolean_new(FALSE));
		update_for_drain(jp, p, start);
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
#line 713 "json_parser.c"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _json_parser_actions + _json_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 12:
#line 143 "json_parser.rl"
	{
		handle_error(jp, JSON_PARSE_ERROR);
		{p++; goto _out; }
	}
	break;
#line 735 "json_parser.c"
		}
	}
	}

	_out: {}
	}

#line 309 "json_parser.rl"
	
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