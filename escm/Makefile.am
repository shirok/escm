# Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

SUBDIRS = src test doc

README : $(top_srcdir)/doc/readme.html
	$(LYNX) -dump $< > $@

$(top_srcdir)/doc/readme.html :
	cd $(top_srcdir)/doc && make readme.html

# end of Makefile.am
