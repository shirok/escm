# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = $(top_builddir)/src/aescm

SUFFIXES = .escm .html

$(ESCM) :
	cd $(top_builddir)/src/ && make

config-h.scm : $(top_builddir)/src/config.h $(ESCM)
	$(AWK) -f $(top_srcdir)/doc/config-h.awk $< > $@

%.html : %.escm config-h.scm $(top_srcdir)/doc/helper.scm
	-LANG=C && $(ESCM) -e "(define *date* \"`date`\")" -e "(define *pwd* \"`pwd`\")" -o $@ $<

EXTRA_DIST = config-h.awk helper.scm @ESCM_DOC_FILES@

CLEANFILES = config-h.scm @HTML_DOC_FILES@

noinst_DATA = $(CLEANFILES)

TESTS =

# end of doc/Makefile.am

