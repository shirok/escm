# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = $(top_builddir)/src/aescm

SUFFIXES = .escm .html

man_MANS = aescm.1

$(ESCM) :
	cd $(top_builddir)/src/ && make

$(top_builddir)/doc/config-h.scm : $(top_builddir)/src/config.h $(ESCM)
	$(AWK) -f $(top_srcdir)/doc/config-h.awk $< > $@

%.html : %.escm config-h.scm $(top_srcdir)/doc/helper.scm
	-LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *pwd* \"`pwd`\")" -o $@ $<

aescm.pod : aescm.epod $(top_builddir)/doc/config-h.scm $(top_srcdir)/doc/helper.scm
	$(ESCM) -e "(define *date* \"`date`\")" -e "(define *pwd* \"`pwd`\")" -o $@ $<

aescm.1 : aescm.pod
	pod2man -c "" -r "" $< > $@

aescm.html : aescm.pod
	pod2html $< > $@

EXTRA_DIST = config-h.awk helper.scm @ESCM_DOC_FILES@ aescm.epod

CLEANFILES = config-h.scm @HTML_DOC_FILES@ aescm.pod aescm.1 aescm.html

noinst_DATA = $(CLEANFILES)

clean-local:
	-rm *~

# end of doc/Makefile.am

