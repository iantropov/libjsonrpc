# Process this file with autoconf to produce a configure script.

# Prelude.
AC_INIT([JSON-RPC], [0.2], [antropov.ivan@gmail.com])

# fairly severe build strictness
# change foreign to gnu or gnits to comply with gnu standards
AM_INIT_AUTOMAKE([-Wall -Werror foreign 1.9.6])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# This macro is defined in check.m4 and tests if check.h and
# libcheck.a are installed in your system. It sets CHECK_CFLAGS and
# CHECK_LIBS accordingly.  
#  AM_PATH_CHECK([MINIMUM-VERSION,
#                [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
AM_PATH_CHECK()

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h event.h])

# Checks for typedefs, structures, and compiler characteristics.

#AC_CHECK_LIB(event, event_init)

# Checks for library functions.
AC_FUNC_MALLOC

# Output files
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/json/Makefile
                 src/json_parser/Makefile
                 src/json_rpc/Makefile
                 src/util/Makefile
                 tests/Makefile
                 tests/json/Makefile
                 tests/json_parser/Makefile
                 tests/json_rpc/Makefile])

AC_OUTPUT
