## Process this file with automake to produce Makefile.in

TESTS = check_koko
check_PROGRAMS = check_koko
check_koko_SOURCES = check_koko.c $(top_builddir)/src/koko/koko.h
check_koko_CFLAGS = @CHECK_CFLAGS@
check_koko_LDADD = $(top_builddir)/src/koko/libkoko.la @CHECK_LIBS@
