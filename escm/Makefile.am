# Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = m4 po src test doc

README : doc/readme.html
	$(LYNX) -dump $< > $@

doc/readme.html :
	cd doc && make readme.html

# This file is required by autoreconf and automake.
# Please `touch' it before rerunning autoreconf.
CLEANFILES = README

# Added by *gettextize*

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath

# end of Makefile.am
