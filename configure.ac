#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([mer], [0.1], [superheron AT gmail DOT com])
AC_CONFIG_SRCDIR([src/main.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([mer], [0.1])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([PALUDIS], [paludis])
AC_SUBST(PALUDIS_CFLAGS)
AC_SUBST(PALUDIS_LIBS)
# Checks for header files.
AC_CHECK_HEADERS([fcntl.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
