/*
 * log.c
 *
 *  Created on: Mar 25, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void log_error(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	vfprintf(stderr, fmt, arg_list);
	va_end(arg_list);

	exit(EXIT_FAILURE);
}

void log_warn(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	vfprintf(stderr, fmt, arg_list);
	va_end(arg_list);
}

void log_info(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	vfprintf(stdout, fmt, arg_list);
	va_end(arg_list);
}
