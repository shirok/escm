# configure.ac
# $Id$

AC_INIT([aescm], [0.16a], [tagga@tsuda.ac.jp])
AM_INIT_AUTOMAKE([aescm], [0.16a])
AM_CONFIG_HEADER([src/config.h])

# Checks for programs.
AC_PROG_CC
CFLAGS="$CFLAGS -Wall"
AC_SUBST(CFLAGS)
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
# AC_PROG_RANLIB
AC_PROG_LIBTOOL
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
AC_CHECK_FUNCS([strchr strerror strstr dup2])

# Filter
AC_ARG_ENABLE([filter],
   [  --enable-filter -- make the filter tool (yes)])
if test x$enable_filter != xno; then
  ESCM=$PACKAGE
  AC_SUBST(ESCM)
fi

# gimmick
AC_OBJEXT
AC_ARG_ENABLE([gimmick],
   [  --enable-gimmick -- enable a gimmick feature in the filter tool (no).])
if test x$enable_gimmick = xyes; then
  GIMMICK_OBJ=lang.$OBJEXT
  AC_SUBST(GIMMICK_OBJ)
fi
AM_CONDITIONAL(GIMMICK, test "$enable_gimmick" = "yes")

# Action Handler
AC_ARG_ENABLE([handler],
  [  --enable-handler -- make an action handler (no).])
if test x$enable_handler != x && test x$enable_handler != xno; then
  if test x$enable_handler = xyes; then
    if test -d /var/www/cgi-bin; then
      CGIDIR=/var/www/cgi-bin
    elif test -d /usr/www/cgi-bin; then
      CGIDIR=/usr/www/cgi-bin
    elif test -d /usr/local/www/cgi-bin; then
      CGIDIR=/usr/local/www/cgi-bin
    elif test -d /usr/apache/cgi-bin; then
      CGIDIR=/usr/apache/cgi-bin
    elif test -d /usr/local/apache/cgi-bin; then
      CGIDIR=/usr/local/apache/cgi-bin
    else
      AC_MSG_ERROR([Can't gusss cgi-bin directory.])
    fi
  else
    CGIDIR=$enable_handler
  fi
  AC_SUBST(CGIDIR)
  ESCM_HANDLER="$PACKAGE-handler"
  AC_SUBST(ESCM_HANDLER)
fi

# Documents
ESCM_DOC_FILES="`cd doc && echo -n *.escm`"
HTML_DOC_FILES=`echo $ESCM_DOC_FILES | sed -e "s/\.escm/.html/g"`
AC_SUBST(ESCM_DOC_FILES)
AC_SUBST(HTML_DOC_FILES)

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile test/Makefile])
AC_OUTPUT
