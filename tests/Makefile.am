## Process this file with automake to produce Makefile.in

SUBDIRS= json .
TESTS = check_lib
check_PROGRAMS = check_lib
check_lib_SOURCES = 	check_lib.c
check_lib_CFLAGS = @CHECK_CFLAGS@
check_lib_LDADD = $(top_builddir)/src/json/libjson.la  $(top_builddir)/tests/json/libcheckjson.la @CHECK_LIBS@
