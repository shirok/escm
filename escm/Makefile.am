# Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = src test doc

README : doc/readme.html
	$(LYNX) -dump $< > $@

doc/readme.html :
	cd doc && make readme.html

# This file is required by autoreconf and automake.
# Please `touch' it before rerunning autoreconf.
CLEANFILES = README

# end of Makefile.am
