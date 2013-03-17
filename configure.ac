#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([path], [0.1.0], [greg@gtgross.com])
AM_INIT_AUTOMAKE(path, 0.1.0)
AC_CONFIG_SRCDIR([src/path.c])
AC_CONFIG_HEADERS(config.h)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_MODE_T

# enable per target flags in Makefile.am
AM_PROG_CC_C_O

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd mkdir realpath strdup])

AC_OUTPUT(Makefile src/Makefile)
