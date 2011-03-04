## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libcheckjsonparser.la
libcheckjsonparser_la_SOURCES = check_json_parser.h \
			check_json_parser.c \
			common.h \
			check_json_parser_parse.h \
			check_json_parser_parse.c \
			$(top_builddir)/src/json_parser/json_parser.h
libcheckjsonparser_la_LIBADD = $(top_builddir)/src/json_parser/libjsonparser.la @CHECK_LIBS@
