dnl acinclude.m4
dnl $Id$
dnl Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

AC_DEFUN([AC_ESCM_CHECK_SCM],
[dnl
if test "$with_scm" = "no"; then
  AC_MSG_ERROR([Required a scheme interpreter.])
fi
if test "$with_scm" = "yes" -o "$with_scm" = ""; then
  with_scm=
  AC_CHECK_PROGS(with_scm, $1)
fi
scm_path=`echo $with_scm | sed -e "s/ .*//"`
scm_args=`echo $with_scm | sed -e "s/^[[^ ]]*//"`
# add the default arguments.
if test "$scm_path" = "gosh" -a "$scm_args" = ""; then
  scm_args=" -b"
elif test "$scm_path" = "guile" -a "$scm_args" = ""; then
  scm_args=" -s /dev/stdin"
elif test "$scm_path" = "rep" -a "$scm_args" = ""; then
  scm_args=" -s /dev/stdin"
fi
scm_path=`which $scm_path`
AC_CHECK_FILE($scm_path)
scm_prog="$scm_path$scm_args"
AC_DEFINE_UNQUOTED(ESCM_SCM, ["$scm_prog"],
	[Command line to invoke the scheme interpreter.])
AC_MSG_CHECKING([for its goodness for escm])
escm_ret=`echo "\"string\"" | env -i $scm_prog 2>&1`
if test x$? != x0 || test "$escm_ret" != ""; then
   AC_MSG_RESULT([fail (code).])
else
   AC_MSG_RESULT([good (code).])
fi
AC_MSG_CHECKING([for its goodness for escm])
escm_ret=`echo "(display \"string\")" | env -i $scm_prog 2>&1`
if test x$? != x0 || test "$escm_ret" != "string"; then
   AC_MSG_RESULT([fail (display and literal).])
else
   AC_MSG_RESULT([good (display and literal).])
fi
scm_args=`echo $scm_prog | sed -e "s/  */\", \"/g"`
AC_DEFINE_UNQUOTED(ESCM_SCM_ARGV, ["$scm_args"], [argv for a Scheme interpreter])
])


dnl end of acinclude.m4
