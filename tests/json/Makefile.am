## Process this file with automake to produce Makefile.in

TESTS = check_json
check_PROGRAMS = check_json
check_json_SOURCES = 	check_json.c \
			common.h \
			check_json_new.h \
			check_json_new.c \
			check_json_array.h \
			check_json_array.c \
			check_json_misc.h \
			check_json_misc.c \
			check_json_object.h \
			check_json_object.c \
			$(top_builddir)/src/json/json.h
check_json_CFLAGS = @CHECK_CFLAGS@
check_json_LDADD = $(top_builddir)/src/json/libjson.la @CHECK_LIBS@
