# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

ESCM = ../src/aescm
POD2MAN = pod2man
POD2HTML = pod2html
SUFFIXES = .html .epod .escm .pod .1

man_MANS = aescm.1 2aescm.1

HTML_FILES = gimmick.html interpreter.html readme.html install.html

JA_FILLES = readme.ja.html install.ja.html

$(ESCM) :
	cd ../src/ && make

doc : $(man_MANS) $(HTML_FILES) $(JA_FILES)

.epod.1 :
	$(ESCM) -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" $< > $*.pod
	$(POD2HTML) $*.pod > $*.html
	$(POD2MAN) -c "" -r "" $*.pod > $@

.escm.html :
	LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" -e "(define *language* 'en)" $< > $@

readme.ja.html : readme.escm
	$(ESCM) -e "(define *date* \"`date`\")" -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" -e "(define *language* 'ja)" $(top_srcdir)/doc/readme.escm > readme.ja.html
	w3m -no-graph -dump readme.html > ../README
	w3m -no-graph -dump readme.ja.html > ../README.ja

install.ja.html : install.escm
	$(ESCM) -e "(define *date* \"`date`\")" -e "(define *builddir* \"$(top_builddir)\")" -e "(define *srcdir* \"$(top_srcdir)\")" -e "(define *language* 'ja)" $(top_srcdir)/doc/install.escm > install.ja.html
	w3m -no-graph -dump install.html > ../INSTALL
	w3m -no-graph -dump install.ja.html > ../INSTALL.ja

EXTRA_DIST = helper.scm
EXTRA_DIST += readme.escm gimmick.escm interpreter.escm install.escm
EXTRA_DIST += aescm.epod 2aescm.epod
EXTRA_DIST += config.scm.in
EXTRA_DIST += $(HTML_FILES) readme.ja.html install.ja.html
EXTRA_DIST += $(man_MANS)

CLEANFILES = 

noinst_DATA = $(HTML_FILES)

clean-local:
	-rm *~
	-rm *.pod

realclean : clean-local
	-rm *.1
	-rm *.html

distclean : realclean

# end of doc/Makefile.am

