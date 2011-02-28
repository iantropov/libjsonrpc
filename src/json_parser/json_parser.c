
#line 1 "json_parser.rl"
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



#line 130 "json_parser.rl"



#line 40 "json_parser.c"
static const char _json_parser_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5, 1, 6, 1, 
	7, 1, 8, 1, 9, 1, 10, 1, 
	11, 1, 12, 1, 13, 2, 6, 3, 
	2, 6, 4, 2, 7, 3, 2, 7, 
	4, 2, 8, 3, 2, 8, 4, 2, 
	9, 3, 2, 9, 4, 2, 10, 3, 
	2, 10, 4, 2, 13, 5, 2, 13, 
	11
};

static const short _json_parser_key_offsets[] = {
	0, 0, 9, 19, 21, 31, 33, 35, 
	36, 45, 51, 57, 63, 69, 71, 77, 
	79, 84, 88, 90, 93, 94, 95, 96, 
	97, 98, 99, 100, 101, 102, 103, 104, 
	105, 106, 109, 119, 121, 131, 133, 135, 
	137, 147, 157, 159, 169, 171, 173, 176, 
	178, 187, 193, 199, 205, 211, 213, 221, 
	223, 230, 234, 236, 241, 242, 243, 244, 
	245, 248, 249, 250, 251, 254, 255, 256, 
	257, 260, 269, 275, 281, 287, 293, 304, 
	314, 316, 326, 328, 330, 333, 343, 345, 
	353, 355, 362, 366, 368, 373, 374, 375, 
	376, 377, 380, 381, 382, 383, 386, 387, 
	388, 389, 392, 401, 407, 413, 419, 425, 
	425, 425
};

static const char _json_parser_trans_keys[] = {
	34, 45, 91, 102, 110, 116, 123, 48, 
	57, 34, 92, -64, -33, -32, -17, -16, 
	-9, 32, 126, -128, -65, 34, 92, -64, 
	-33, -32, -17, -16, -9, 32, 126, -128, 
	-65, -128, -65, 0, 34, 47, 92, 98, 
	102, 110, 114, 116, 117, 48, 57, 65, 
	70, 97, 102, 48, 57, 65, 70, 97, 
	102, 48, 57, 65, 70, 97, 102, 48, 
	57, 65, 70, 97, 102, 48, 57, 0, 
	46, 69, 101, 48, 57, 48, 57, 0, 
	69, 101, 48, 57, 43, 45, 48, 57, 
	48, 57, 0, 48, 57, 97, 108, 115, 
	101, 0, 117, 108, 108, 0, 114, 117, 
	101, 0, 32, 34, 125, 34, 92, -64, 
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
	102, 0
};

static const char _json_parser_single_lengths[] = {
	0, 7, 2, 0, 2, 0, 0, 1, 
	9, 0, 0, 0, 0, 0, 4, 0, 
	3, 2, 0, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 3, 2, 0, 2, 0, 0, 2, 
	8, 2, 0, 2, 0, 0, 3, 2, 
	9, 0, 0, 0, 0, 0, 6, 0, 
	5, 2, 0, 3, 1, 1, 1, 1, 
	3, 1, 1, 1, 3, 1, 1, 1, 
	3, 9, 0, 0, 0, 0, 9, 2, 
	0, 2, 0, 0, 3, 8, 0, 6, 
	0, 5, 2, 0, 3, 1, 1, 1, 
	1, 3, 1, 1, 1, 3, 1, 1, 
	1, 3, 9, 0, 0, 0, 0, 0, 
	0, 0
};

static const char _json_parser_range_lengths[] = {
	0, 1, 4, 1, 4, 1, 1, 0, 
	0, 3, 3, 3, 3, 1, 1, 1, 
	1, 1, 1, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 4, 1, 4, 1, 1, 0, 
	1, 4, 1, 4, 1, 1, 0, 0, 
	0, 3, 3, 3, 3, 1, 1, 1, 
	1, 1, 1, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 3, 3, 3, 3, 1, 4, 
	1, 4, 1, 1, 0, 1, 1, 1, 
	1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 3, 3, 3, 3, 0, 
	0, 0
};

static const short _json_parser_index_offsets[] = {
	0, 0, 9, 16, 18, 25, 27, 29, 
	31, 41, 45, 49, 53, 57, 59, 65, 
	67, 72, 76, 78, 81, 83, 85, 87, 
	89, 91, 93, 95, 97, 99, 101, 103, 
	105, 107, 111, 118, 120, 127, 129, 131, 
	134, 144, 151, 153, 160, 162, 164, 168, 
	171, 181, 185, 189, 193, 197, 199, 207, 
	209, 216, 220, 222, 227, 229, 231, 233, 
	235, 239, 241, 243, 245, 249, 251, 253, 
	255, 259, 269, 273, 277, 281, 285, 296, 
	303, 305, 312, 314, 316, 320, 330, 332, 
	340, 342, 349, 353, 355, 360, 362, 364, 
	366, 368, 372, 374, 376, 378, 382, 384, 
	386, 388, 392, 402, 406, 410, 414, 418, 
	419, 420
};

static const unsigned char _json_parser_indicies[] = {
	0, 2, 4, 5, 6, 7, 8, 3, 
	1, 13, 14, 9, 10, 11, 12, 1, 
	15, 1, 19, 20, 16, 17, 18, 15, 
	1, 16, 1, 17, 1, 22, 21, 15, 
	15, 15, 15, 15, 15, 15, 15, 23, 
	1, 24, 24, 24, 1, 25, 25, 25, 
	1, 26, 26, 26, 1, 15, 15, 15, 
	1, 27, 1, 28, 29, 30, 30, 27, 
	21, 31, 1, 32, 30, 30, 31, 21, 
	33, 33, 34, 1, 34, 1, 32, 34, 
	21, 35, 1, 36, 1, 37, 1, 38, 
	1, 39, 21, 40, 1, 41, 1, 42, 
	1, 43, 21, 44, 1, 45, 1, 46, 
	1, 47, 21, 48, 49, 50, 21, 55, 
	56, 51, 52, 53, 54, 21, 57, 21, 
	61, 62, 58, 59, 60, 57, 21, 58, 
	21, 59, 21, 63, 64, 21, 64, 65, 
	66, 68, 69, 70, 71, 72, 67, 21, 
	77, 78, 73, 74, 75, 76, 21, 79, 
	21, 83, 84, 80, 81, 82, 79, 21, 
	80, 21, 81, 21, 85, 86, 50, 21, 
	86, 49, 21, 79, 79, 79, 79, 79, 
	79, 79, 79, 87, 21, 88, 88, 88, 
	21, 89, 89, 89, 21, 90, 90, 90, 
	21, 79, 79, 79, 21, 91, 21, 92, 
	93, 94, 95, 95, 96, 91, 21, 97, 
	21, 98, 99, 95, 95, 100, 97, 21, 
	101, 101, 102, 21, 102, 21, 98, 99, 
	100, 102, 21, 103, 21, 104, 21, 105, 
	21, 106, 21, 107, 108, 109, 21, 110, 
	21, 111, 21, 112, 21, 113, 114, 115, 
	21, 116, 21, 117, 21, 118, 21, 119, 
	120, 121, 21, 57, 57, 57, 57, 57, 
	57, 57, 57, 122, 21, 123, 123, 123, 
	21, 124, 124, 124, 21, 125, 125, 125, 
	21, 57, 57, 57, 21, 126, 127, 128, 
	130, 131, 132, 133, 134, 135, 129, 21, 
	140, 141, 136, 137, 138, 139, 21, 142, 
	21, 146, 147, 143, 144, 145, 142, 21, 
	143, 21, 144, 21, 148, 149, 131, 21, 
	149, 127, 128, 130, 132, 133, 134, 135, 
	129, 21, 150, 21, 151, 152, 153, 154, 
	155, 154, 150, 21, 156, 21, 157, 158, 
	154, 159, 154, 156, 21, 160, 160, 161, 
	21, 161, 21, 157, 158, 159, 161, 21, 
	162, 21, 163, 21, 164, 21, 165, 21, 
	166, 167, 168, 21, 169, 21, 170, 21, 
	171, 21, 172, 173, 174, 21, 175, 21, 
	176, 21, 177, 21, 178, 179, 180, 21, 
	142, 142, 142, 142, 142, 142, 142, 142, 
	181, 21, 182, 182, 182, 21, 183, 183, 
	183, 21, 184, 184, 184, 21, 142, 142, 
	142, 21, 21, 21, 21, 0
};

static const char _json_parser_trans_targs[] = {
	2, 0, 13, 14, 7, 20, 25, 29, 
	7, 3, 5, 6, 4, 7, 8, 4, 
	3, 5, 6, 7, 8, 0, 111, 9, 
	10, 11, 12, 14, 111, 15, 17, 16, 
	111, 18, 19, 21, 22, 23, 24, 111, 
	26, 27, 28, 111, 30, 31, 32, 111, 
	33, 34, 112, 35, 37, 38, 36, 39, 
	73, 36, 35, 37, 38, 39, 73, 39, 
	40, 41, 53, 54, 46, 60, 65, 69, 
	46, 42, 44, 45, 43, 46, 48, 43, 
	42, 44, 45, 46, 48, 46, 47, 49, 
	50, 51, 52, 54, 46, 47, 55, 57, 
	112, 56, 46, 47, 112, 58, 59, 61, 
	62, 63, 64, 46, 47, 112, 66, 67, 
	68, 46, 47, 112, 70, 71, 72, 46, 
	47, 112, 74, 75, 76, 77, 78, 79, 
	86, 87, 84, 113, 93, 98, 102, 84, 
	80, 82, 83, 81, 84, 106, 81, 80, 
	82, 83, 84, 106, 84, 85, 87, 84, 
	85, 88, 90, 113, 89, 84, 85, 113, 
	91, 92, 94, 95, 96, 97, 84, 85, 
	113, 99, 100, 101, 84, 85, 113, 103, 
	104, 105, 84, 85, 113, 107, 108, 109, 
	110
};

static const char _json_parser_trans_actions[] = {
	0, 0, 27, 27, 5, 0, 0, 0, 
	3, 27, 27, 27, 27, 62, 27, 0, 
	0, 0, 0, 23, 0, 25, 0, 0, 
	0, 0, 0, 0, 15, 0, 0, 0, 
	13, 0, 0, 0, 0, 0, 0, 21, 
	0, 0, 0, 17, 0, 0, 0, 19, 
	0, 0, 7, 27, 27, 27, 27, 59, 
	27, 0, 0, 0, 0, 11, 0, 0, 
	0, 0, 27, 27, 5, 0, 0, 0, 
	3, 27, 27, 27, 27, 62, 27, 0, 
	0, 0, 0, 23, 0, 0, 0, 0, 
	0, 0, 0, 0, 15, 15, 0, 0, 
	35, 0, 13, 13, 29, 0, 0, 0, 
	0, 0, 0, 21, 21, 53, 0, 0, 
	0, 17, 17, 41, 0, 0, 0, 19, 
	19, 47, 0, 0, 0, 0, 0, 0, 
	27, 27, 5, 9, 0, 0, 0, 3, 
	27, 27, 27, 27, 62, 27, 0, 0, 
	0, 0, 23, 0, 0, 0, 0, 15, 
	15, 0, 0, 38, 0, 13, 13, 32, 
	0, 0, 0, 0, 0, 0, 21, 21, 
	56, 0, 0, 0, 17, 17, 44, 0, 
	0, 0, 19, 19, 50, 0, 0, 0, 
	0
};

static const char _json_parser_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 1, 
	0, 0, 0, 0, 0, 0, 1, 0, 
	1, 0, 0, 1, 0, 0, 0, 0, 
	1, 0, 0, 0, 1, 0, 0, 0, 
	1, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0
};

static const char _json_parser_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 25, 
	0, 0, 0, 0, 0, 0, 25, 0, 
	25, 0, 0, 25, 0, 0, 0, 0, 
	25, 0, 0, 0, 25, 0, 0, 0, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 25, 
	25, 25, 25, 25, 25, 25, 25, 0, 
	0, 0
};

static const int json_parser_start = 1;
static const int json_parser_first_final = 111;
static const int json_parser_error = 0;

static const int json_parser_en_object = 33;
static const int json_parser_en_array = 78;
static const int json_parser_en_main = 1;


#line 133 "json_parser.rl"

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
			 	
	
#line 443 "json_parser.c"
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
	case 1:
#line 40 "json_parser.rl"
	{
		tmp = json_object_new();
		ret = begin(&jp, tmp);
		if (ret == -1)
			{p++; goto _out; }
		jp.cur_obj = tmp;
		jp.add = object_add;
		
		{stack[top++] = cs; cs = 33; goto _again;}
	}
	break;
	case 2:
#line 51 "json_parser.rl"
	{
		tmp = json_array_new();
		ret = begin(&jp, tmp);
		if (ret == -1)
			{p++; goto _out; }
		jp.cur_obj = tmp;
		jp.add = array_add;
		
		{stack[top++] = cs; cs = 78; goto _again;}
	}
	break;
	case 3:
#line 62 "json_parser.rl"
	{
		parser_pop_context(&jp);
		{cs = stack[--top]; goto _again;}
	}
	break;
	case 4:
#line 67 "json_parser.rl"
	{
		parser_pop_context(&jp);
		{cs = stack[--top]; goto _again;}
	}
	break;
	case 5:
#line 72 "json_parser.rl"
	{
		jp.p_name = get_interprete_str(&jp, p, buf);
		ret = jp.p_name == NULL ? -1 : 0;
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 6:
#line 79 "json_parser.rl"
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_double_new(atof(buf + jp.p_start)));
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 7:
#line 85 "json_parser.rl"
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_int_new(atoi(buf + jp.p_start)));
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 8:
#line 91 "json_parser.rl"
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_null_new());
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 9:
#line 97 "json_parser.rl"
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_boolean_new(TRUE));
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 10:
#line 103 "json_parser.rl"
	{
		ret = jp.add(jp.cur_obj, &jp.p_name, json_boolean_new(FALSE));
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 11:
#line 109 "json_parser.rl"
	{
		tmp_str = get_interprete_str(&jp, p, buf);
		ret = jp.add(jp.cur_obj, &jp.p_name, json_string_new(tmp_str));
		free(tmp_str);		
		if (ret == -1)
			{p++; goto _out; }
	}
	break;
	case 12:
#line 117 "json_parser.rl"
	{
		{p++; goto _out; }
	}
	break;
	case 13:
#line 122 "json_parser.rl"
	{
		jp.p_start = (int)(p - buf);
	}
	break;
#line 628 "json_parser.c"
		}
	}

_again:
	_acts = _json_parser_actions + _json_parser_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 0:
#line 36 "json_parser.rl"
	{
		is_end = 1;
	}
	break;
#line 643 "json_parser.c"
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
#line 117 "json_parser.rl"
	{
		{p++; goto _out; }
	}
	break;
#line 664 "json_parser.c"
		}
	}
	}

	_out: {}
	}

#line 238 "json_parser.rl"

	if (ret == -1 || is_end == 0) {
		json_parser_destroy(&jp);
		return NULL;
	}
	
	struct json_object *res_obj = json_array_get(jp.cur_obj, 0);
	json_ref_get(res_obj);
	
	json_parser_destroy(&jp);
	
	return res_obj;
}
