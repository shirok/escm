# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = lang

# Program
BUILT_SOURCES = deflang.c
CLEANFILES = $(BUILT_SOURCES) langconf$(EXEEXT) aescm.cgi$(EXEEXT) fescm.cgi$(EXEEXT)

noinst_PROGRAMS = langconf
langconf_SOURCES = langconf.c escm.c escm.h lang.c

deflang.c : langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ 
	./langconf$(EXEEXT) $(srcdir)/lang/@DEFAULT_LANG@ > $@

bin_PROGRAMS = aescm
aescm_SOURCES  = filter.c escm.c escm.h lang.c

DEFS = -DESCM_LANG_DIR=\"$(pkgdatadir)/lang/\" @DEFS@


# CGI
EXTRA_PROGRAMS = aescm.cgi fescm.cgi
CGI_SOURCES = escmcgi.c escmcgi.h $(BUILT_SOURCES)
aescm_cgi_SOURCES = cgimain.c escm.c escm.h $(CGI_SOURCES)
fescm_cgi_SOURCES = fifo.c escm.c escm.h $(CGI_SOURCES)

# end of src/Makefile.am
