/*
 * log.c
 *
 *  Created on: Mar 25, 2011
 *      Author: ant
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_ERROR_LEVEL "error"
#define LOG_WARN_LEVEL "warn"
#define LOG_INFO_LEVEL "info"
#define LOG_LIBRARY_PREFIX "libjsonrpc"

static void log_helper(FILE *out, char *level, char *fmt, va_list arg_list)
{
	char buf[1024];

	if (fmt != NULL)
		vsprintf(buf, fmt, arg_list);
	else
		buf[0] = '\0';

	fprintf(out, "[%s_%s]: %s\n", LOG_LIBRARY_PREFIX, level, buf);
}

void log_error(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	log_helper(stderr, LOG_ERROR_LEVEL, fmt, arg_list);
	va_end(arg_list);

	exit(EXIT_FAILURE);
}

void log_warn(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	log_helper(stderr, LOG_WARN_LEVEL, fmt, arg_list);
	va_end(arg_list);
}

void log_info(char *fmt, ...)
{
	va_list arg_list;

	va_start(arg_list, fmt);
	log_helper(stdout, LOG_INFO_LEVEL, fmt, arg_list);
	va_end(arg_list);
}
