# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = deflang.c langconf$(EXEEXT) aescm.cgi$(EXEEXT) fescm.cgi$(EXEEXT)

noinst_PROGRAMS = langconf
langconf_SOURCES = langconf.c escm.c escm.h lang.c

deflang.c : langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ 
	./langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ > $@

bin_PROGRAMS = aescm
aescm_SOURCES  = escm.c escm.h filter.c lang.c $(BUILT_SOURCES)

DEFS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\" @DEFS@


# CGI
EXTRA_PROGRAMS = aescm.cgi fescm.cgi
aescm_cgi_SOURCES = cgimain.c escm.c escm.h escmcgi.c escmcgi.h $(BUILT_SOURCES)
fescm_cgi_SOURCES = fifo.c escm.c escm.h escmcgi.c escmcgi.h $(BUILT_SOURCES)

# end of src/Makefile.am
