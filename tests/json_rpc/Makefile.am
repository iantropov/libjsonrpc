## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libcheckjsonrpc.la
libcheckjsonrpc_la_SOURCES = check_json_rpc.h \
			check_json_rpc.c \
			common.h \
			check_json_rpc_process.h \
			check_json_rpc_process.c \
			$(top_builddir)/src/json_rpc/json_rpc.h
libcheckjsonrpc_la_LIBADD = $(top_builddir)/src/json_rpc/libjsonrpc.la @CHECK_LIBS@
#libcheckjsonrpc_la_CFLAGS = '-I/usr/local/include -L/usr/local/lib'
#libcheckjsonrpc_la_LDFLAGS = '-levent'

