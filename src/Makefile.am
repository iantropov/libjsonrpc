## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libjsonrpc.la

json_parser.c : json_parser.rl
	ragel -o $@ $<

libjsonrpc_la_SOURCES = json.c json.h \
			json_parser.h json_parser.c \
			json_rpc.h json_rpc.c json_rpc_tt.c \
			list.h string_functions.c string_functions.h \
			log.h log.c \
			json_rpc_tt_ws.c \
			json_rpc_tt_tcp.c \
			json_rpc_tt_http.c

library_includedir=$(includedir)/json_rpc
library_include_HEADERS = json.h json_parser.h json_rpc.h
