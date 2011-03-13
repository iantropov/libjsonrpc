## Process this file with automake to produce Makefile.in

bin_PROGRAMS = main
main_SOURCES = main.c
main_LDADD = 	json/libjson.la \
		json_parser/libjsonparser.la \
		json_rpc/libjsonrpc.la \
		util/libutil.la \
		https/libhttps.la

SUBDIRS=json util json_parser json_rpc https .
