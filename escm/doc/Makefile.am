# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = ../src/aescm

SUFFIXES = .escm .html

man_MANS = aescm.1

ESCM_DOC_FILES = readme.escm gimmick.escm interpreter.escm
HTML_DOC_FILES = readme.html gimmick.html interpreter.html

$(ESCM) :
	cd ../src/ && make

config-h.scm : ../src/config.h $(ESCM)
	$(AWK) -f $(top_srcdir)/doc/config-h.awk $< > $@

%.html : %.escm config-h.scm $(top_srcdir)/doc/helper.scm
	-LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(load \"$(top_srcdir)/doc/helper.scm\")" -o $@ $<

aescm.pod : aescm.epod config-h.scm $(top_srcdir)/doc/helper.scm
	$(ESCM) -e "(load \"$(top_srcdir)/doc/helper.scm\")" -o $@ $<

aescm.1 : aescm.pod
	$(POD2MAN) -c "" -r "" $< > $@

aescm.html : aescm.pod
	$(POD2HTML) $< > $@

EXTRA_DIST = config-h.awk helper.scm $(ESCM_DOC_FILES) aescm.epod

CLEANFILES = config-h.scm $(HTML_DOC_FILES) aescm.pod aescm.1 aescm.html

noinst_DATA = $(CLEANFILES)

clean-local:
	-rm *~

# end of doc/Makefile.am

