dnl acinclude.m4
dnl $Id$
dnl Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

AC_DEFUN([AC_ESCM_CHECK_BACKEND],
[dnl
if test "$with_backend" = "no"; then
  AC_MSG_ERROR([The backend interpreter required.])
fi
if test "$with_backend" = "yes" -o "$with_backend" = ""; then
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
dnl end of acinclude.m4
