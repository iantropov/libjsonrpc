## Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libjsonparser.la
json_parser.c : json_parser.rl
	ragel -o $@ $<

libjsonparser_la_SOURCES = 	json_parser.h \
				json_parser.c