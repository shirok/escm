# configure.ac
# $Id$

AC_INIT([aescm], [0.18], [tagga@tsuda.ac.jp])
AM_INIT_AUTOMAKE([aescm], [0.18])
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

# Checks for Scheme.
AC_ARG_WITH([scm], [  --with-scm="scheme arg ... "   specify the Scheme interpreter (gosh).])
AC_ESCM_CHECK_SCM([gosh guile rep])

# Checks for libraries.
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])
# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
# Checks for library functions.
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([strchr strerror strstr dup2])

# gimmick
AC_OBJEXT
AC_ARG_ENABLE([gimmick],
   [  --enable-gimmick -- enable a gimmick feature in the filter tool (no).])
if test x$enable_gimmick = xyes; then
  GIMMICK_OBJ=lang.$OBJEXT
  AC_SUBST(GIMMICK_OBJ)
fi
AM_CONDITIONAL(GIMMICK, test "$enable_gimmick" = "yes")

# Documents
ESCM_DOC_FILES="`cd doc && echo -n *.escm`"
HTML_DOC_FILES=`echo $ESCM_DOC_FILES | sed -e "s/\.escm/.html/g"`
AC_SUBST(ESCM_DOC_FILES)
AC_SUBST(HTML_DOC_FILES)

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile test/Makefile])
AC_OUTPUT
