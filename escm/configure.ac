# configure.ac
# $Id$

AC_INIT
AM_INIT_AUTOMAKE(aescm, 0.15)
AM_CONFIG_HEADER(src/config.h)

# check for programs
AC_PROG_RANLIB
AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_CHECK_PROGS(LYNX, [lynx w3m-en links w3m])

# check for header files
AC_STDC_HEADERS
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([dup2 strerror])


# Multiple language support
EXTRA_OBJS=
## Set the default language
AC_ARG_WITH(default-lang,
	[  --with-default-lang=LANG   Set the default language (scm)])
if test x$with_default_lang == xyes || test x$with_default_lang == x; then
   with_default_lang=scm
elif test x$with_default_lang == xno; then
   AC_MSG_ERROR([Need a default language])
elif ! test -f "src/data/"$with_default_lang; then
   AC_MSG_ERROR([Language not supported: $with_default_lang])
fi
AC_DEFINE_UNQUOTED(ESCM_DEFAULT_LANGPTR, (&escm_${with_default_lang}),
					 [ptr to the default language])
AC_DEFINE_UNQUOTED(ESCM_DEFAULT_LANG, $with_default_lang,
				      	 [the default language])

## Configurations for the Scheme language
if test "x$with_default_lang" == "xscm" && test "x$with_scm" == "x"; then
   with_scm=yes
fi
escm_path=
escm_argv=
if test "x$with_scm" != "xno" && test "x$with_scm" != "x"; then
   if expr index "$with_scm"  "/" > /dev/null; then
       escm_argv="$with_scm"
   elif expr index "$with_scm" " " > /dev/null; then
       escm_argv="$with_scm"      
   else
       if test "x$with_scm" == "xyes"; then
	  AC_CHECK_PROGS(ESCM_SCM, [gosh stklos guile rep])
	  with_scm=$ESCM_SCM
       fi
       AC_PATH_PROG(escm_path, $with_scm)
       if test x$escm_path == x; then
	  AC_MSG_ERROR([Scheme interpretor not found: $with_scm])
       fi
       if test -f scm/$with_scm.sh; then
            . "scm/$with_scm.sh"
       fi
       if test "x$ac_scm_args" != "x"; then
	  with_scm="$escm_path $ac_scm_args"
       else
	  with_scm=$escm_path
       fi
   fi
fi

# Configurations for optional languages
escm_langlist=
for file in `echo src/data/*`; do
     lang=`expr substr $file 10 16`
     if eval test "\"X\$with_"$lang"\"" != "X" && eval test "\"X\$with_"$lang"\"" != "Xno"; then
	escm_langlist=$escm_langlist" "$lang
     fi
done
if test "x$escm_langlist" == "x"; then
   AC_MSG_ERROR([No script language incorporated])
fi

# Generate src/langconf.h
escm_outfile="src/langconf.h"
echo "#ifndef LANGCONF_H" > $escm_outfile
echo "#define LANGCONF_H 1" >> $escm_outfile
for lang in $escm_langlist; do
    EXTRA_OBJS=$EXTRA_OBJS" "$lang".\$(OBJEXT)"
    eval escm_argv="\$with_$lang"
    escm_tmp=`echo $lang | tr "a-z" "A-Z"`
    if test "$escm_argv" != "yes"; then
       escm_argv=`echo $escm_argv | sed -e "s/  */\", \"/"`
       echo "#define ESCM_ARGV_"$escm_tmp" \""$escm_argv"\"" >> $escm_outfile
    else
       echo "#undef ESCM_ARGV_"$escm_tmp >> $escm_outfile
    fi
done
for lang in $escm_langlist; do
    echo "extern struct escm_lang escm_"$lang";" >> $escm_outfile
done
echo "#ifdef IN_MAIN_C" >> $escm_outfile
echo "#ifdef ESCM_PLURISCRIPT" >> $escm_outfile
echo "static struct escm_lang * const lang[[]] = {" >> $escm_outfile
for lang in $escm_langlist; do
    echo "  &escm_"$lang"," >> $escm_outfile
done
echo "};" >> $escm_outfile
echo "#endif /* ESCM_PLURISCRIPT */" >> $escm_outfile
echo "#endif /* IN_MAIN_C */" >> $escm_outfile
echo "#endif /* LANGCONF_H */" >> $escm_outfile

EXTRA_OBJS=`echo $EXTRA_OBJS | sed -e "s/^ //"`
AC_SUBST(EXTRA_OBJS)
if test 0 != `expr index "$EXTRA_OBJS" " "`; then
   AC_DEFINE(ESCM_PLURISCRIPT, 1, [Support plural scripts])
   opt_L="[[-L language]]"
   opt_l="l"
fi

# Other configurations
AC_DEFINE(ESCM_N_EXPR, 10, [The maximum number of -e options])

# Enable optional syntaxes
AC_ARG_WITH(default-style,
	[  --with-default-style=STYLE   Set the default style (xmlpi)])
case x$with_default_style in
xyes|x) with_default_style=xmlpi;;
xno) AC_MSG_ERROR([Need a default style]);;
xxmlpi|xarchaic|xscript) ;;
*) AC_MSG_ERROR([Style not supported: $with_default_style]);;
esac
AC_DEFINE_UNQUOTED(ESCM_DEFAULT_STYLEPTR, (&escm_${with_default_style}_style),
					 [The default preprocessor])
AC_DEFINE_UNQUOTED(ESCM_DEFAULT_STYLE, $with_default_style,
				       [The default style])

AC_ARG_ENABLE(xmlpi,
	[  --enable-xmlpi   Support the XML style <?LANG ... ?> (yes)])
AC_ARG_ENABLE(archaic,
	[  --enable-archaic   Support the archaic style <? expr ... !> (yes)])
AC_ARG_ENABLE(script,
	[  --enable-script    Support the script style <% expr ... %> (yes)])
escm_disabled=
case x$enable_xmlpi in
xno) escm_disabled=xmlpi;;
x*) opt_x=x; AC_DEFINE(ESCM_ENABLE_XMLPI, 1, [support the xmlpi style]);;
esac 
case x$enable_archaic in
xno) escm_dispaled=$escm_disabled" archaic";;
x*) opt_a=a; AC_DEFINE(ESCM_ENABLE_ARCHAIC, 1, [support the archaic style]);;
esac 
case x$enable_script in
xno) escm_disabled=$escm_disabled" script";;
x*) opt_s=s; AC_DEFINE(ESCM_ENABLE_SCRIPT, 1, [support the script style]);;
esac

if expr "$escm_disabled" : $with_default_style > /dev/null; then
   AC_MSG_ERROR([Can't disable the default syntax: $with_default_style])
fi

AC_DEFINE_UNQUOTED(ESCM_USAGE, ["[[-EHS${opt_a}${opt_l}${opt_s}v${opt_x}]]${opt_L}[[-i interpreter]][[-e expr ...]] file ...\n"],
			       [Short help message])

# Create Makefiles.
AC_CONFIG_FILES([Makefile
	src/Makefile
	doc/Makefile])
AC_OUTPUT
# end of configure.ac
