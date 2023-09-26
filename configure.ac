#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.71])
AC_INIT([irssi-python],[test5],[staff@irssi.org])
AC_CONFIG_SRCDIR([src/pycore.c])
AC_CONFIG_HEADERS([pyirssi-config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC([])
LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([PYTHON], [python3-embed])
PKG_CHECK_MODULES([IRSSI], [irssi-1 >= 1.4.4])
AM_PATH_GLIB_2_0(2.0.0) 

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strchr strrchr])

IRSSI_PYTHON_INCLUDES="${PYTHON_CFLAGS} ${IRSSI_CFLAGS}"

AC_SUBST([IRSSI_PYTHON_INCLUDES])

IRSSI_MODULEDIR=$( $PKG_CONFIG --variable=irssimoduledir irssi-1 )
IRSSI_SIGNALSFILE=$( $PKG_CONFIG --variable=signalsfile irssi-1 )

AC_SUBST([IRSSI_MODULEDIR])
AC_SUBST([IRSSI_SIGNALSFILE])

CFLAGS="$CFLAGS -Wall -fno-strict-aliasing"

AC_CONFIG_FILES([Makefile scripts/Makefile  src/Makefile src/objects/Makefile])
AC_OUTPUT
