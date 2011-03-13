## Process this file with automake to produce Makefile.in

SUBDIRS= json json_parser json_rpc .
TESTS = check_lib
check_PROGRAMS = check_lib
check_lib_SOURCES = 	check_lib.c
check_lib_CFLAGS = @CHECK_CFLAGS@
check_lib_LDADD = 	$(top_builddir)/src/libjsonrpc.la  \
			$(top_builddir)/tests/json/libcheckjson.la \
			$(top_builddir)/tests/json_parser/libcheckjsonparser.la \
			$(top_builddir)/tests/json_rpc/libcheckjsonrpc.la \
			 @CHECK_LIBS@