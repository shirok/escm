# configure.ac
# $Id$

AC_INIT([aescm], [0.31], [tagga@tsuda.ac.jp])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
CFLAGS="$CFLAGS -Wall"
AC_SUBST(CFLAGS)
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK

# The default interpreter language
AC_ARG_WITH([default], [  --with-default=lang   specify the default language (scm).])
if test x$with_default == xyes || test x$with_default == x; then
   DEFAULT_LANG=scm
elif test x$with_default == xno; then
   AC_MSG_ERROR([The default language required.])
else
   DEFAULT_LANG=$with_default
fi
AC_SUBST(DEFAULT_LANG)
AC_DEFINE_UNQUOTED(DEFAULT_LANG, ["$DEFAULT_LANG"], ["the default language"])

AC_ARG_WITH([scheme], AS_HELP_STRING([--with-scheme], [the default scheme program and its arguments (gosh -b)]))
if test "$with_scheme" = "yes" || test "$with_scheme" = ""; then
   SCHEME="gosh -b"
elif test "$with_default" = "no"; then
   SCHEME="guile -s /dev/stdin"
else
   SCHEME="$with_scheme"
fi
AC_SUBST(SCHEME)

# Checks for the backend interpreter.
AC_ARG_WITH([backend], [  --with-backend="interpreter arg ... "   specify the backend interpreter (gosh).])
AC_ESCM_CHECK_BACKEND([gosh guile])

# Checks for libraries.
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h strings.h unistd.h])
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strchr strtol getopt_long])

AC_CONFIG_FILES([Makefile test/Makefile
			  doc/Makefile doc/config.scm
			  src/Makefile src/lang/Makefile src/lang/scm])
AC_OUTPUT
