# doc/Makefile.am
# $Id$
# Author: TAGA Yoshitaka <tagga@tsuda.ac.jp>

all : $(htm_DATA) $(man_MANS)

ESCM = $(top_builddir)/src/aescm -l $(top_srcdir)/src/lang/scm -i "gosh -b -I."
POD2MAN = pod2man
POD2HTML = pod2html
HTMLDUMP = w3m -dump
SUFFIXES = .1 .html .epod .pod .escm

man_MANS = aescm.1

.epod.pod :
	$(ESCM) $< > $*.pod

.pod.1 :
	$(POD2MAN) -c "" -r "" $< > $@

.pod.html :
	$(POD2HTML) $< > $*.html

htmldir = $(datadir)/doc/$(PACKAGE)-$(VERSION)
html_DATA = gimmick.html interpreter.html readme.html install.html cgi.html

.escm.html :
	LANG=C && $(ESCM) -e "(define *date* \"`date -u`\")" -e "(define *language* 'en)" $< > $@

document : $(man_MANS) $(html_DATA)
	$(HTMLDUMP) readme.html > ../README
	$(HTMLDUMP) install.html > ../INSTALL

EXTRA_DIST = $(man_MANS:.1=.epod) $(man_MANS:.1=.html) $(html_DATA:.html=.escm)
EXTRA_DIST += $(man_MANS) $(html_DATA) helper.scm config.scm.in

clean-local:
	-rm *~
	-rm *.tmp
	-rm *.pod

distclean-am : clean-local
	-rm $(man_MANS) $(html_DATA) aescm.html

# end of doc/Makefile.am

