# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = deflang.c
deflang.c : lang/@DEFAULT_LANG@ lang.awk
	$(AWK) -f lang.awk $< > $@

bin_PROGRAMS = aescm
aescm_SOURCES = escm.c escm.h cgi.c fork.c $(BUILT_SOURCES)
aescm_SOURCES += filter.c meta_arg.c meta_arg.h
aescm_SOURCES += lang.awk

# other language support
aescm_LDADD = @POLYGLOT_OBJ@
aescm_DEPENDENCIES = @POLYGLOT_OBJ@
EXTRA_aescm_SOURCES = lang.c

if POLYGLOT
AM_CFLAGS = -DESCM=1 -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\"
pkgdata_DATA = lang.awk
else
AM_CFLAGS = -DESCM=1
endif



# handler CGI program
if HANDLER
cgibindir = @CGIBIN@
cgibin_PROGRAMS = aescm
endif

# end of src/Makefile.am
