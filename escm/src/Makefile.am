# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = deflang.c langconf$(EXEEXT)

noinst_PROGRAMS = langconf aescm.cgi aescm.fcgi tiny.fcgi
langconf_SOURCES = langconf.c escm.c escm.h lang.c

deflang.c : langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ 
	./langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ > $@

bin_PROGRAMS = aescm
aescm_SOURCES  = escm.c escm.h filter.c $(BUILT_SOURCES)

# other language support
aescm_LDADD = @POLYGLOT_OBJ@
aescm_DEPENDENCIES = @POLYGLOT_OBJ@
EXTRA_aescm_SOURCES = lang.c

DEFS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\" -DESCM_PATH=\"$(bindir)/$(PACKAGE)\" @DEFS@


# CGI
aescm_cgi_SOURCES = cgimain.c escm.c escm.h $(BUILT_SOURCES)

aescm_fcgi_SOURCES = fastcgi.c escm.c escm.h $(BUILT_SOURCES)

# end of src/Makefile.am
