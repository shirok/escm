# configure.ac
# $Id$

AC_INIT([aescm], [0.21], [tagga@tsuda.ac.jp])
AM_INIT_AUTOMAKE([aescm], [0.21])
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
CFLAGS="$CFLAGS -Wall"
AC_SUBST(CFLAGS)
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
# AC_PROG_RANLIB
# AC_PROG_LIBTOOL
AC_CHECK_PROGS(LYNX, [lynx links w3m])

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

# Checks for the backend interpreter.
AC_ARG_WITH([backend], [  --with-backend="interpreter arg ... "   specify the backend interpreter (gosh).])
AC_ESCM_CHECK_BACKEND([gosh guile])

# Checks for libraries.
# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_PID_T
# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([atexit dup2 strerror strchr strtol])

# polyglot
AC_OBJEXT
AC_ARG_ENABLE([polyglot],
   [  --enable-polyglot -- enable polyglot support (no).])
if test x$enable_polyglot = xyes; then
  POLYGLOT_OBJ=lang.$OBJEXT
  AC_SUBST(POLYGLOT_OBJ)
  AC_DEFINE_UNQUOTED(ENABLE_POLYGLOT, [1], [enable polyglot support.])
fi
AM_CONDITIONAL(POLYGLOT, test "$enable_polyglot" = "yes")
LANG_FILES="`cd src/lang && echo -n [[a-z]]*`"
AC_SUBST(LANG_FILES)

# CGI program
AC_ARG_ENABLE([cgi],
   [  --enable-cgi -- use as a CGI program (yes).])
AC_ESCM_CHECK_CGI

# Documents
ESCM_DOC_FILES="`cd doc && echo -n *.escm`"
HTML_DOC_FILES=`echo $ESCM_DOC_FILES | sed -e "s/\.escm/.html/g"`
AC_SUBST(ESCM_DOC_FILES)
AC_SUBST(HTML_DOC_FILES)

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile test/Makefile src/lang/Makefile])
AC_OUTPUT
