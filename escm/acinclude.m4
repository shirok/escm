dnl acinclude.m4
dnl $Id$
dnl Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# --with-backend="prog arg ..."
AC_DEFUN([AC_ESCM_CHECK_BACKEND],
[dnl
if test "$with_backend" = "no"; then
  AC_MSG_ERROR([The backend interpreter required.])
fi
if test "$with_backend" = "yes" || test "$with_backend" = ""; then
  with_backend=
  AC_CHECK_PROGS(with_backend, $1)
fi
backend_path=`echo $with_backend | sed -e "s/ .*//"`
backend_args=`echo $with_backend | sed -e "s/^[[^ ]]*//"`
# add the default arguments.
if test "$backend_path" = "gosh" -a "$backend_args" = ""; then
  backend_args=" -b"
elif test "$backend_path" = "guile" -a "$backend_args" = ""; then
  backend_args=" -s /dev/stdin"
fi
backend_path=`which $backend_path`
AC_CHECK_FILE($backend_path)
backend_prog="$backend_path$backend_args"
AC_DEFINE_UNQUOTED(ESCM_BACKEND, ["$backend_prog"],
	[Command line to invoke the backend interpreter.])
backend_args=`echo $backend_prog | sed -e "s/  */\", \"/g"`
AC_DEFINE_UNQUOTED(ESCM_BACKEND_ARGV, ["$backend_args"], [argv for the backend interpreter])
])

# --enable-handler
AC_DEFUN([AC_ESCM_CHECK_HANDLER],
[dnl
ENABLE_HANDLER=1
if test x$enable_handler = xno; then
   enable_handler=no
   ENABLE_HANDLER=
elif test x$enable_handler = xyes || test x$enable_handler = x; then
#   if test -d $prefix/public_html; then
#      CGIBIN=$prefix/public_html/cgi-bin
#   elif test -d $prefix/cgi-bin; then
   if test -d $prefix/cgi-bin; then
      CGIBIN=$prefix/cgi-bin
   else
      for x in /Local/Library/WebServer/CGI-Executables /Library/WebServer/CGI-Executables /opt/apache/share/cgi-bin /boot/home/apache/cgi-bin /usr/local/apache/cgi-bin /usr/local/httpd/cgi-bin /usr/local/www/cgi-bin /usr/local/share/apache/cgi-bin /usr/share/apache/cgi-bin /var/apache/cgi-bin /var/www/cgi-bin; do
	  if test -d $x; then
	     CGIBIN=$x
	     break
          fi
      done
   fi
else
   CGIBIN=$enable_handler
fi
AC_SUBST(CGIBIN)
AM_CONDITIONAL(HANDLER, test "$enable_handler" != "no")
if test x$ENABLE_HANDLER != x; then
   AC_DEFINE_UNQUOTED(ENABLE_HANDLER, [1], ["Whether to use it as a handler CGI program."])
fi
])
dnl end of acinclude.m4
