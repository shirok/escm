# src/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

# Common part
noinst_LIBRARIES = libescm.a
libescm_a_SOURCES = escm.c cgi.c escm.h

# Scheme specific
scm_sources = escm_scm.c escm_scm.h

# Programs
EXTRA_PROGRAMS = aescm aescm-handler

# filter tool
bin_PROGRAMS = @ESCM@
aescm_SOURCES = filter.c meta_arg.c meta_arg.h $(scm_sources)
aescm_LDADD = libescm.a @GIMMICK_OBJ@
aescm_DEPENDENCIES = @GIMMICK_OBJ@
EXTRA_aescm_SOURCES = lang.c
if GIMMICK
AM_CFLAGS = -DESCM_LANG_DIR=\"$(datadir)/lang\"
endif
EXTRA_DIST = lang/

# action handler
cgidir = @CGIDIR@
cgi_PROGRAMS = @ESCM_HANDLER@
aescm_handler_SOURCES = handler.c $(scm_sources)
aescm_handler_LDADD = libescm.a

install-data-local :
if GIMMICK
	-$(INSTALL) -d $(pkgdatadir)/lang
	$(INSTALL_DATA) lang/* $(pkgdatadir)/lang
endif

# end of src/Makefile.am
