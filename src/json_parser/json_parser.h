/*
 * json_parser.h
 *
 *  Created on: Feb 27, 2011
 *      Author: ant
 */

#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include "../json/json.h"
#include <stdlib.h>

struct json_parser;

struct json_parser *json_parser_init();
void json_parser_reset(struct json_parser *jp);
int json_parser_parse(struct json_parser *jp, char *str, size_t length);
struct json_object *json_parser_get_json(struct json_parser *jp);
void json_parser_destroy(struct json_parser *jp);


#endif /* JSON_PARSER_H_ */
