#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ(2.59)
AC_INIT([irssi-python], [test5], [loafier@gmail.com])
AC_CONFIG_SRCDIR([src/pycore.c])
AC_CONFIG_HEADER([pyirssi-config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AM_DISABLE_STATIC
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([PYTHON], [python3-embed])
PKG_CHECK_MODULES([IRSSI], [irssi-1])
AM_PATH_GLIB_2_0(2.0.0) 

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([memset strchr strrchr])

IRSSI_PYTHON_INCLUDES="${PYTHON_CFLAGS} ${IRSSI_CFLAGS}"

AC_SUBST([IRSSI_PYTHON_INCLUDES])

dnl * gcc specific options
if test "x$ac_cv_prog_gcc" = "xyes"; then
    CFLAGS="$CFLAGS -Wall -fno-strict-aliasing"
fi

#AC_CONFIG_FILES([src/Makefile src/objects/Makefile])
#AC_OUTPUT([Makefile src/Makefile src/objects/Makefile])
#AC_OUTPUT([Makefile src/Makefile src/objects/Makefile])
AC_CONFIG_FILES([Makefile scripts/Makefile  src/Makefile src/objects/Makefile])
AC_OUTPUT
