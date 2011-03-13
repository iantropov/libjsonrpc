## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libjsonrpc.la

json_parser.c : json_parser.rl
	ragel -o $@ $<

libjsonrpc_la_SOURCES = http.c \
						json.c json.h \
						json_parser.h json_parser.c \
						json_rpc.h json_rpc.c json_rpc_over_http.c \
						list.h string_functions.c string_functions.h

library_includedir=$(includedir)/json_rpc
library_include_HEADERS = json.h json_parser.h json_rpc.h https.h
