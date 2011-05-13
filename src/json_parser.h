/*
 * json_parser.h
 *
 *  Created on: Feb 27, 2011
 *      Author: ant
 */

#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include "json.h"
#include <stdlib.h>

#define JSON_PARSE_ERROR 0x1
#define JSON_INTERNAL_ERROR 0x2

struct json_parser;

typedef void (*json_success_cb)(struct json_parser *jp, struct json_object *obj, void *arg);
typedef void (*json_error_cb)(struct json_parser *jp, short error, void *arg);

struct json_parser *json_parser_new(json_success_cb s_cb, json_error_cb e_cb, void *arg);
void json_parser_free(struct json_parser *jp);
void json_parser_process(struct json_parser *jp, char *start, char *end);
int json_parser_drain(struct json_parser *jp);

struct json_object *json_parser_parse(char *str);


#endif /* JSON_PARSER_H_ */
